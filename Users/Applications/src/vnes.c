#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\build.h"
#include "..\..\config.h"
#include "..\..\gui\inc\ui_resources.h"
#include "..\..\gui\inc\ui_keyboard.h"
#include "..\..\gui\inc\ui_chart.h"
#include "..\..\gui\inc\ui_button.h"
#include "..\..\gui\inc\ui_panel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jansson.h"
#if SHARD_SSL_SUPPORT
#include "wolfssl\version.h"
#endif
#include "libpng\png.h"
#include "zlib\zlib.h"
#include "jconfig.h"
#include <math.h>
#include "efat.h"

extern const unsigned char g_fm_appicon_png48[1345];
extern const unsigned char g_vnes_appicon_png48[1836];

extern const unsigned char g_fm_file_png32[1380];
extern const unsigned char g_fm_folder_png32[1672] ;
extern const unsigned char g_fm_sdcard_png32[1429];
extern const unsigned char g_fm_fdrive_png32[1785];
extern const unsigned char g_fm_picfile_png32[1995];

extern uint8 core_init(uchar* buffer, int len);
extern uchar core_exec(uchar* vbuffer);
extern void ppu_render(uchar* output);


typedef struct emuscreen_object emuscreen_object;
struct emuscreen_object {
	ui_object base;
	uint32 framebuffer_size;
	uint32 framebuffer_pxfmt;
	uint32 * framebuffer;
};

static void vnes_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(vnes_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	//uint32 longtime = 0;
	//ui_object * screen;
	ui_object * cfBtn;
	//tk_clear_subconfig_menu(ctx);
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "vnes"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "vnesApp") ;
	if(cfBtn != NULL) {
		((ui_icon_tool *)cfBtn)->show_marker = FALSE;
	}
	//ui_clear_dispstack(ctx->display);
	//tk_clear_body(ctx);
	OS_DEBUG_EXIT();
}

typedef struct ui_fileitem ui_fileitem;
struct ui_fileitem {
	ui_item base;
	efat_FileSystem * fs;
	char path[256];
} ;

extern ui_fileitem * ui_create_diritem(char * name, efat_FileSystem * fs, char * path);
extern ui_fileitem * ui_create_fileitem(char * name, efat_FileSystem * fs, char * path, void (* click)(ui_object * obj, void * params));
void vnes_directory_click(ui_object * obj, void * params);
void vnes_file_click(ui_object * obj, void * params);
void ui_listfileitem_render(ui_object * obj, gui_handle_p display);
uint8 vnes_check_fs(dev_node * root, efat_FileSystem * fs);


void ui_emuscreen_render(struct ui_object * obj, gui_handle_p display) {
	ui_buffer srcbuf;
	ui_rect srcrect;
	ui_buffer dstbuf;
	ui_rect dstrect;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	//uint16 w = ((ui_rect *)obj)->w;
	//uint16 h = ((ui_rect *)obj)->h;
	if(obj->state & UI_STATE_INIT) {
		srcrect.x = 0;
		srcrect.y = 0;
		srcrect.h = 240;
		srcrect.w = 256;
		srcbuf.buffer = (uint8 *)((emuscreen_object *)obj)->framebuffer;
		srcbuf.length = ((emuscreen_object *)obj)->framebuffer_size;
		srcbuf.pxfmt = ((emuscreen_object *)obj)->framebuffer_pxfmt;
		srcbuf.size.w = 256;
		srcbuf.size.h = 240;
		
		dstbuf.buffer = display->fb_ptr;
		dstbuf.size.w = display->width;
		dstbuf.size.h = display->height;
		dstrect.x = x;
		dstrect.y = y;
		dstrect.w = 256;
		dstrect.h = 240;
		ui_rect_copy(&srcbuf, &srcrect, &dstbuf, &dstrect, UI_RGB565); 
	}
}


ui_object * ui_create_emuscreen(uint16 x, uint16 y, uint16 w, uint16 h) {
	ui_object * obj = (ui_object *)ui_create_object(sizeof(emuscreen_object) , UI_TYPE_LABEL | UI_TYPE_PANEL | UI_ALIGN_FLOAT) ;
	((ui_object *)obj)->rect.x = x;
	((ui_object *)obj)->rect.y = y;
	((ui_object *)obj)->rect.w = w;
	((ui_object *)obj)->rect.h = h;
	//use RGB565 for framebuffer
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = 0;
	((ui_object *)obj)->render = ui_emuscreen_render;
	((ui_object *)obj)->handler = NULL;
	((emuscreen_object *)obj)->framebuffer_size = (w * h * sizeof(uint16));
	((emuscreen_object *)obj)->framebuffer_pxfmt = UI_RGB565;
	((emuscreen_object *)obj)->framebuffer = os_alloc(w * h * sizeof(uint32));
	return (ui_object *)obj;
}

static void ui_listfileitem_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_listfileitem_render);
	//uint16  wtxt, xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 stat;
	if(obj->state != 0) {
		stat = ui_item_render(obj, display);
		display->set_area(display, x, y, w, h);
		ui_item_text_render(obj, display, stat);
	}
	OS_DEBUG_EXIT();
}

#define VNES_STATE_READY			0
#define VNES_STATE_PLAY				1
#define VNES_STATE_PAUSE			2

uint8 _vnes_state = VNES_STATE_READY;

static void vnes_task() {
	OS_DEBUG_ENTRY(vnes_task);
	tk_context_p ctx;
	//ui_object * obj;
	//uint8 orientation = 0;
	//uint16 i, j=0;
	uint8 ret;
	uint32 psw;
	//ui_chart * chart = NULL;
	//ui_series * spal_perf = NULL;
	//ui_infostat * infostat;
	emuscreen_object * emuscreen;
	int render_counter = 0;
	ctx = os_get_context();
	
	while(1) {
		switch(_vnes_state) {
			case VNES_STATE_READY: os_wait(200); break;
			case VNES_STATE_PLAY: 
				emuscreen = (emuscreen_object *)ui_get_object_by_name(ctx->display, "emuscreen");
				if(emuscreen != NULL) {
					psw = os_enter_critical();
					while(_vnes_state == VNES_STATE_PLAY) {
						ret = core_exec((uchar *)emuscreen->framebuffer);
						if(ret) {
							render_counter ++;
							if(render_counter == 7) {
								ppu_render((uchar *)emuscreen->framebuffer);
								os_exit_critical(psw);
								ui_reinitialize_object((ui_object *)emuscreen);
								os_wait(75);
								psw = os_enter_critical();
								render_counter = 0;
							}
						}
					}
					os_exit_critical(psw);
				} else {
					_vnes_state = VNES_STATE_PAUSE;
				}
				break;
			case VNES_STATE_PAUSE:
				os_wait(200);
				break;
		}
	}
	OS_DEBUG_EXIT();
}


void vnes_directory_click(ui_object * obj, void * params) {
	efat_DirList * dir = NULL;
	uint16 i = 0;
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * panel = NULL;
	//ui_fileitem * root = ((ui_fileitem *)obj);
	ui_object * item;
	char path[256];
	if(dev_get_node_by_driver(ctx->devices, ((ui_fileitem *)obj)->fs) == NULL) return;
	dir = dir_CreateEntryList((uchar *)((ui_fileitem *)obj)->path, (fat_FileSystem *)((ui_fileitem *)obj)->fs);
	panel = ui_get_object_by_name(params, "rom_panel");
	if(panel == NULL) return;
	
	ui_list_clear(params, (ui_list_scroll *)panel);
	//create up directory
	snprintf(path, 256, "%s", ((ui_fileitem *)obj)->path);
	for(i=strlen(path)-2;i>0;i--) {
		if(path[i] == '\\') break;
		path[i] = 0;
	}
	ui_list_add(panel, (item = (ui_object *)ui_create_diritem((char *)"..", (fat_FileSystem *)((ui_fileitem *)obj)->fs, path)));
	ui_set_target(item, ctx);
	//create subfolders/subfiles
	while(dir_GetEntryList(dir) != 0) {
		if(strcmp((const char *)dir->entry->lfn, "..") == 0 || strcmp((const char *)dir->entry->lfn, ".") == 0) continue; 
		if(dir->entry->attrib & FAT_ATTR_DIR) {
			snprintf(path, 256, "%s%s\\", ((ui_fileitem *)obj)->path, dir->entry->lfn);
			ui_list_add(panel, (item = (ui_object *)ui_create_diritem((char *)dir->entry->lfn, (fat_FileSystem *)((ui_fileitem *)obj)->fs, path)));
			ui_set_target(item, ctx);
		} else {
			snprintf(path, 256, "%s%s", ((ui_fileitem *)obj)->path, dir->entry->lfn);
			ui_list_add(panel, (item = (ui_object *)ui_create_fileitem((char *)dir->entry->lfn, (fat_FileSystem *)((ui_fileitem *)obj)->fs, path, vnes_file_click)));
			ui_set_target(item, ctx);
		}
	}
	dir_DeleteList(dir);
	ui_reinitialize_object(panel);
}

static uint8 * _codespace = NULL;

void vnes_file_click(ui_object * obj, void * params) {
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * panel = NULL;
	uint16 str_index;
	//ui_fileitem * root = ((ui_fileitem *)obj);
	//ui_object * item;
	efat_File * nes_file;
	uint32 len;
	uint32 index = 0;
	char ext[5];
	char buffer[256];
	char * path;
	emuscreen_object * emuscreen;
	if(dev_get_node_by_driver(ctx->devices, ((ui_fileitem *)obj)->fs) == NULL) return;
	
	panel = ui_get_object_by_name(params, "rom_panel");
	if(panel == NULL) return;
	path = ((ui_fileitem *)obj)->path;
	//open file at path
	//check if file extension .nes
	str_index = strlen(path) - 4;
	strncpy(ext, path + str_index, 5);
	if(strcmp(ext, ".nes") == 0) {
		//nes file matched
		len = 0;
		index = 0;
		if(_codespace == NULL) _codespace = os_alloc(512 * 1024);		//allocate 512KB for nes file buffer
		else {
			os_free(_codespace);
			_codespace = os_alloc(512 * 1024);		//re-allocate 512KB for nes file buffer
		}
		nes_file = efat_Open((uchar *)path, ((ui_fileitem *)obj)->fs, (uchar *)"rw+");
		while((len = efat_Read(sizeof(buffer), nes_file, (uchar *)buffer)) == sizeof(buffer)) {
			memcpy(_codespace + index, buffer, len);
			index += len;
		}
		memcpy(_codespace + index, buffer, len);
		index += len;
		emuscreen = (emuscreen_object *)ui_get_object_by_name(params, "emuscreen");
		//start initialize core6502
		if(core_init(_codespace, index)) {
			_vnes_state = VNES_STATE_PLAY;		//this method must exit
		}
	}
}

static void vnes_drive_click(ui_object * obj, void * params) {
	efat_DirList * dir = NULL;
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * panel = NULL;
	ui_object * item ;
	char path[256];
	//load file system
	dev_node * sdcard = dev_get_node_by_name(ctx->devices, (char *)((ui_text *)obj)->text);
	if(sdcard != NULL) {
		dir = dir_CreateEntryList((uchar *)"\\", (fat_FileSystem *)sdcard->driver);
		panel = ui_get_object_by_name(params, "rom_panel");
		if(panel == NULL) return;
		ui_list_clear(params, (ui_list_scroll *)panel);
		//iterate subfolders
		while(dir_GetEntryList(dir) != 0) {
			if(dir->entry->attrib & FAT_ATTR_DIR) {
			snprintf(path, 256, "%s%s\\", "\\", dir->entry->lfn);
				ui_list_add(panel, (item = (ui_object *)ui_create_diritem((char *)dir->entry->lfn, (fat_FileSystem *)sdcard->driver, path)));
				ui_set_target(item, ctx);
			} else {
			snprintf(path, 256, "%s%s", "\\", dir->entry->lfn);
				ui_list_add(panel, (item = (ui_object *)ui_create_fileitem((char *)dir->entry->lfn, (fat_FileSystem *)sdcard->driver, path, vnes_file_click)));
				ui_set_target(item, ctx);
			}
		}
		dir_DeleteList(dir);
		ui_reinitialize_object(panel);
	}
}

static void vnes_show(tk_context_p ctx) {
	OS_DEBUG_ENTRY(vnes_show);
	//ui_toggle * play;
	//ui_datetime * dtime;
	//datetime dval;
	//ui_object * list;
	//uint32 longtime;
	ui_object * obj;
	//ui_object * calc_keypad1, * calc_keypad2;
	uint16 keypad_height = 0;
	//uint16 y = 0;
	dev_node * iterator;
	dev_node * sub_iterator;
	dev_node * root;
	//tk_config * conf = ctx->config;
	//ui_object * lcd;
	//ui_chart * chart;
	ui_object * panel;
	ui_object * emuscreen;
	ui_object * item;
	//ui_series * series;
	os_task * vnestask = NULL;
	//dir_DirList * dir;
	//ui_rect rect = { 0, 0, 48, 48 };
	//ui_object * btn1, * btn2, * btn3;
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	//ui_clear_dispstack(ctx->display);
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	obj = ui_get_object_by_name(ctx->display, "vnesApp") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "vnes") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "vnes");
	ui_set_text((ui_text *)screen, (uint8 *)"vnes");
	//ui_add_body(ctx->display, (panel = ui_panel_create(UI_ALIGN_VERTICAL, 96, 32)));
	ui_add_body(ctx->display, (emuscreen = ui_create_emuscreen(0, 0, 256, 240)));
	ui_set_object_name(emuscreen, "emuscreen");
	ui_set_align(emuscreen, UI_ALIGN_HORIZONTAL);
	ui_add_body(ctx->display, (panel = ui_list_create(0, 0, 144, 200)));
	ui_set_object_name(panel, "rom_panel");
	ui_set_align(panel, UI_ALIGN_HORIZONTAL);
	
	root = ctx->devices;
	if(root != NULL) {
		iterator = root->child;
		//also iterate subchild for partition (TODO)
		while(iterator != NULL) {
			//iterate sub devices for some type
			switch((iterator->type & (DEV_TYPE_MASK | DEV_SUBTYPE_MASK))) {
				case DEV_TYPE_SDIO:
					//start iterate sub child (check for sdcard partition)
					sub_iterator = iterator->child;
					while(sub_iterator != NULL) {
						if((sub_iterator->type & DEV_TYPE_MASK) == DEV_TYPE_STORAGE) {
							//sd card
							ui_list_add(panel, (item = ui_item_create((uchar *)sub_iterator->name, (uint8 *)g_fm_sdcard_png32, sizeof(g_fm_sdcard_png32), 1, vnes_drive_click)));
							ui_set_target(item, ctx);
						}
						sub_iterator = sub_iterator->sibling;
					}
					break;
				case DEV_TYPE_USBH:	
					//start iterate sub child (check for thumbdrive partition)
					sub_iterator = iterator->child;
					while(sub_iterator != NULL) {
						if((sub_iterator->type & DEV_TYPE_MASK) == DEV_TYPE_STORAGE) {
							//flash drive
							ui_list_add(panel, (item = ui_item_create((uchar *)sub_iterator->name, (uint8 *)g_fm_fdrive_png32, sizeof(g_fm_fdrive_png32), 1, vnes_drive_click)));
							ui_set_target(item, ctx);
						}
						sub_iterator = sub_iterator->sibling;
					}
					break;
			}
			//next device
			iterator = iterator->sibling;
		}
		ui_reinitialize_object(panel);
	}
	vnestask = os_find_task_by_name("vnestask");			//spectrum analyzer task
	if(vnestask == NULL) {
		//memset(g_spectral_buffer, 0, sizeof(g_spectral_buffer));		//clear performance buffer
		vnestask = os_create_task(ctx, vnes_task, "vnestask", 27, 65536);		//8K stack size
	}
	
	if(((gui_handle_p)ctx->display)->orientation & 0x01) {
		//ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w, keypad_height / 2, 0)));
		//ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x, y + (keypad_height / 2), screen->rect.w, keypad_height / 2, 1)));
	} else {
		//ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w/2, keypad_height, 0)));
		//ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x + (screen->rect.w/2), y, screen->rect.w/2, keypad_height, 1)));
	}
	
	OS_DEBUG_EXIT();
}

static void vnes_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(vnes_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	//ui_object * instance;
	//ui_textbox * textbox;
	//uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "vnes")) != NULL) {
		if(((gui_handle_p)ctx->display)->body != screen) {
			vnes_exit_click(obj, params);
		} else {
			//ui_remove_screen_unsafe(ctx->display, screen);
			//screen = ui_push_screen(ctx->display, screen);
			//ui_set_object_name(screen, "chart");
			//ui_set_text((ui_text *)screen, (uint8 *)"chart");
		}
	} else {
			vnes_show(ctx);
	}
	OS_DEBUG_EXIT();
	return;
}


void vnes_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(vnes_init);
	ui_object * calc = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (calc = ui_header_create((uint8 *)"vnesApp", (uint8 *)"vnes", header_w, header_w, sizeof(g_vnes_appicon_png48), (uint8 *)g_vnes_appicon_png48, vnes_click)));
		if(calc != NULL) ui_set_target(calc, params);
	}
	OS_DEBUG_EXIT();
}

void vnes_app_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(vnes_app_init);
	ui_object * chart = NULL;
	ui_object * body = handle->body;
	ui_object * obj;
	uint16 header_w = 64;
	if(params != NULL && body != NULL) {
		ui_add_body(handle, (chart = ui_launcher_create((uint8 *)"vnesApp", (uint8 *)"vnes", header_w, header_w, sizeof(g_vnes_appicon_png48), (uint8 *)g_vnes_appicon_png48, vnes_click)));
		if(chart != NULL) {
			obj = ui_get_object_by_name(handle, "vnes") ;
			if(obj != NULL) {
				((ui_icon_tool *)chart)->show_marker = TRUE;
			}
			ui_set_target(chart, params);
		}
	}
	OS_DEBUG_EXIT();
}

