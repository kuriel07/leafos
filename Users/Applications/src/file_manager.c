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

extern const unsigned char g_fm_file_png32[1380];
extern const unsigned char g_fm_folder_png32[1672] ;
extern const unsigned char g_fm_sdcard_png32[1429];
extern const unsigned char g_fm_fdrive_png32[1785];
extern const unsigned char g_fm_picfile_png32[1995];

static void file_manager_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(file_manager_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint32 longtime = 0;
	ui_object * screen;
	ui_object * cfBtn;
	//tk_clear_subconfig_menu(ctx);
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "file_manager"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "fmApp") ;
	if(cfBtn != NULL) {
		((ui_icon_tool *)cfBtn)->show_marker = FALSE;
	}
	//ui_clear_dispstack(ctx->display);
	//tk_clear_body(ctx);
	OS_DEBUG_EXIT();
}


static void fm_task() {
	OS_DEBUG_ENTRY(fm_task);
	tk_context_p ctx;
	ui_object * obj;
	uint8 orientation = 0;
	uint16 i, j=0;
	ui_chart * chart = NULL;
	ui_series * spal_perf = NULL;
	ui_infostat * infostat;
	ctx = os_get_context();
	
	while(1) {
		os_wait(100);		//tick at 1 second
	}
	
	OS_DEBUG_EXIT();
}

typedef struct ui_fileitem ui_fileitem;
struct ui_fileitem {
	ui_item base;
	efat_FileSystem * fs;
	char path[256];
} ;

ui_fileitem * ui_create_diritem(char * name, efat_FileSystem * fs, char * path);
ui_fileitem * ui_create_fileitem(char * name, efat_FileSystem * fs, char * path, void (* click)(ui_object * obj, void * params));
void file_manager_directory_click(ui_object * obj, void * params);
void file_manager_file_click(ui_object * obj, void * params);
void ui_listfileitem_render(ui_object * obj, gui_handle_p display);
uint8 file_manager_check_fs(dev_node * root, efat_FileSystem * fs);

void ui_fileitem_release(void * obj) {
	ui_fileitem * item = (ui_fileitem *)obj;
	//should do operation here
}

ui_fileitem * ui_create_diritem(char * name, efat_FileSystem * fs, char * path) {
	ui_fileitem * item = (ui_fileitem *)ui_create_item(sizeof(ui_fileitem), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, (char *)name);
	((ui_item *)item)->id = 0;
	((ui_object *)item)->rect.h = 48;
	((ui_object *)item)->image.buffer = (uint8 *)g_fm_folder_png32;
	((ui_object *)item)->image.size = sizeof(g_fm_folder_png32);
	((ui_object *)item)->render = ui_listfileitem_render;
	((ui_object *)item)->forecolor = UI_COLOR_WHITE;
	((ui_object *)item)->backcolor = UI_COLOR_WHITE;
	((ui_object *)item)->handler = file_manager_directory_click;
	((ui_fileitem *)item)->fs = fs;
	strncpy(((ui_fileitem *)item)->path, path, 256);
	((ui_object *)item)->release = ui_fileitem_release;
	return item;
}

ui_fileitem * ui_create_fileitem(char * name, efat_FileSystem * fs, char * path, void (* click)(ui_object * obj, void * params)) {
	char ext[5];
	uint16 str_index = 0;
	ui_fileitem * item = (ui_fileitem *)ui_create_item(sizeof(ui_fileitem), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, (char *)name);
	((ui_item *)item)->id = 0;
	((ui_object *)item)->rect.h = 48;
	str_index = strlen(path) - 4;
	strncpy(ext, path + str_index, 5);
	if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".png") == 0) {
		((ui_object *)item)->image.buffer = (uint8 *)g_fm_picfile_png32;
		((ui_object *)item)->image.size = sizeof(g_fm_picfile_png32);
	} else {
		((ui_object *)item)->image.buffer = (uint8 *)g_fm_file_png32;
		((ui_object *)item)->image.size = sizeof(g_fm_file_png32);
	}
	((ui_object *)item)->render = ui_listfileitem_render;
	((ui_object *)item)->forecolor = UI_COLOR_WHITE;
	((ui_object *)item)->backcolor = UI_COLOR_WHITE;
	if(click != NULL) ((ui_object *)item)->handler = click;
	else ((ui_object *)item)->handler = file_manager_file_click;
	((ui_fileitem *)item)->fs = fs;
	strncpy(((ui_fileitem *)item)->path, path, 256);
	((ui_object *)item)->release = ui_fileitem_release;
	return item;
	
}

static void ui_listfileitem_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_listfileitem_render);
	uint16 nc, wtxt, xx, yy;
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


void file_manager_directory_click(ui_object * obj, void * params) {
	efat_DirList * dir = NULL;
	uint16 i = 0;
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * panel = NULL;
	ui_fileitem * root = ((ui_fileitem *)obj);
	ui_object * item;
	char path[256];
	if(dev_get_node_by_driver(ctx->devices, ((ui_fileitem *)obj)->fs) == NULL) return;
	dir = dir_CreateEntryList((uchar *)((ui_fileitem *)obj)->path, (fat_FileSystem *)((ui_fileitem *)obj)->fs);
	panel = ui_get_object_by_name(params, "explorer_panel");
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
		snprintf(path, 256, "%s%s\\", ((ui_fileitem *)obj)->path, dir->entry->lfn);
		if(dir->entry->attrib & FAT_ATTR_DIR) {
			ui_list_add(panel, (item = (ui_object *)ui_create_diritem((char *)dir->entry->lfn, (fat_FileSystem *)((ui_fileitem *)obj)->fs, path)));
			ui_set_target(item, ctx);
		} else {
			ui_list_add(panel, (item = (ui_object *)ui_create_fileitem((char *)dir->entry->lfn, (fat_FileSystem *)((ui_fileitem *)obj)->fs, path, NULL)));
			ui_set_target(item, ctx);
		}
	}
	dir_DeleteList(dir);
	ui_reinitialize_object(panel);
}

void file_manager_file_click(ui_object * obj, void * params) {
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * panel = NULL;
	ui_fileitem * root = ((ui_fileitem *)obj);
	ui_object * item;
	char * path;
	if(dev_get_node_by_driver(ctx->devices, ((ui_fileitem *)obj)->fs) == NULL) return;
	
	panel = ui_get_object_by_name(params, "explorer_panel");
	if(panel == NULL) return;
	path = ((ui_fileitem *)obj)->path;
	//open file at path
	
}

static void file_manager_drive_click(ui_object * obj, void * params) {
	efat_DirList * dir = NULL;
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * panel = NULL;
	ui_object * item ;
	char path[256];
	//load file system
	dev_node * sdcard = dev_get_node_by_name(ctx->devices, (char *)((ui_text *)obj)->text);
	if(sdcard != NULL) {
		dir = dir_CreateEntryList((uchar *)"\\", (fat_FileSystem *)sdcard->driver);
		panel = ui_get_object_by_name(params, "explorer_panel");
		if(panel == NULL) return;
		ui_list_clear(params, (ui_list_scroll *)panel);
		//iterate subfolders
		while(dir_GetEntryList(dir) != 0) {
			snprintf(path, 256, "%s%s\\", "\\", dir->entry->lfn);
			if(dir->entry->attrib & FAT_ATTR_DIR) {
				ui_list_add(panel, (item = (ui_object *)ui_create_diritem((char *)dir->entry->lfn, (fat_FileSystem *)sdcard->driver, path)));
				ui_set_target(item, ctx);
			} else {
				ui_list_add(panel, (item = (ui_object *)ui_create_fileitem((char *)dir->entry->lfn, (fat_FileSystem *)sdcard->driver, path, NULL)));
				ui_set_target(item, ctx);
			}
		}
		dir_DeleteList(dir);
		ui_reinitialize_object(panel);
	}
}

static void file_manager_show(tk_context_p ctx) {
	OS_DEBUG_ENTRY(file_manager_show);
	ui_toggle * play;
	ui_datetime * dtime;
	datetime dval;
	ui_object * list;
	uint32 longtime;
	ui_object * obj;
	ui_object * calc_keypad1, * calc_keypad2;
	uint16 keypad_height = 0;
	uint16 y = 0;
	dev_node * iterator;
	dev_node * sub_iterator;
	dev_node * root;
	tk_config * conf = ctx->config;
	ui_object * lcd;
	ui_chart * chart;
	ui_object * panel;
	ui_object * item;
	ui_series * series;
	os_task * spaltask = NULL;
	dir_DirList * dir;
	ui_rect rect = { 0, 0, 48, 48 };
	ui_object * btn1, * btn2, * btn3;
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	//ui_clear_dispstack(ctx->display);
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	obj = ui_get_object_by_name(ctx->display, "fmApp") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "file_manager") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "file_manager");
	ui_set_text((ui_text *)screen, (uint8 *)"file_manager");
	//ui_add_body(ctx->display, (panel = ui_panel_create(UI_ALIGN_VERTICAL, 96, 32)));
	ui_add_body(ctx->display, (panel = ui_list_create(0, 0, 200, 200)));
	ui_set_object_name(panel, "explorer_panel");
	ui_set_align(panel, UI_ALIGN_FULL);
	
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
							ui_list_add(panel, (item = ui_item_create((uchar *)sub_iterator->name, (uint8 *)g_fm_sdcard_png32, sizeof(g_fm_sdcard_png32), 1, file_manager_drive_click)));
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
							ui_list_add(panel, (item = ui_item_create((uchar *)sub_iterator->name, (uint8 *)g_fm_fdrive_png32, sizeof(g_fm_fdrive_png32), 1, file_manager_drive_click)));
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
	
	if(((gui_handle_p)ctx->display)->orientation & 0x01) {
		//ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w, keypad_height / 2, 0)));
		//ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x, y + (keypad_height / 2), screen->rect.w, keypad_height / 2, 1)));
	} else {
		//ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w/2, keypad_height, 0)));
		//ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x + (screen->rect.w/2), y, screen->rect.w/2, keypad_height, 1)));
	}
	
	OS_DEBUG_EXIT();
}

static void file_manager_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(file_manager_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * instance;
	ui_textbox * textbox;
	uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "file_manager")) != NULL) {
		if(((gui_handle_p)ctx->display)->body != screen) {
			file_manager_exit_click(obj, params);
		} else {
			//ui_remove_screen_unsafe(ctx->display, screen);
			//screen = ui_push_screen(ctx->display, screen);
			//ui_set_object_name(screen, "chart");
			//ui_set_text((ui_text *)screen, (uint8 *)"chart");
		}
	} else {
			file_manager_show(ctx);
	}
	OS_DEBUG_EXIT();
	return;
}


void file_manager_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(file_manager_init);
	ui_object * calc = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (calc = ui_header_create((uint8 *)"fmApp", (uint8 *)"file_manager", header_w, header_w, sizeof(g_fm_appicon_png48), (uint8 *)g_fm_appicon_png48, file_manager_click)));
		if(calc != NULL) ui_set_target(calc, params);
	}
	OS_DEBUG_EXIT();
}

void file_manager_app_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(file_manager_app_init);
	ui_object * chart = NULL;
	ui_object * body = handle->body;
	ui_object * obj;
	uint16 header_w = 64;
	if(params != NULL && body != NULL) {
		ui_add_body(handle, (chart = ui_launcher_create((uint8 *)"fmApp", (uint8 *)"file_manager", header_w, header_w, sizeof(g_fm_appicon_png48), (uint8 *)g_fm_appicon_png48, file_manager_click)));
		if(chart != NULL) {
			obj = ui_get_object_by_name(handle, "file_manager") ;
			if(obj != NULL) {
				((ui_icon_tool *)chart)->show_marker = TRUE;
			}
			ui_set_target(chart, params);
		}
	}
	OS_DEBUG_EXIT();
}