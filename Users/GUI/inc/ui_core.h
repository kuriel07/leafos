/*!\file 		ui_core.h
 * \brief     	ui core windows manager
 * \details   	ui core for managing user interface
 * \author    	AGP
 * \version   	1.0
 * \date      	2016
 * \pre       	
 * must provide display APIs on if_gui_init assigned on gui_handle (such as set_area, print_string, fill_area, fill_bitmap) 
 * each API please refer to documentation
 * future reference when exporting these APIs, use SWI to switch between user mode and kernel mode on ARM (design supported)
\verbatim	
********************************************************************
1.0
 * initial release (2016.03.05)
 * cascaded body and header, body ui_objects can be stacked (stacked windows manager) (2016.03.05)
 * tree based rendering algorithm (2016.03.05)
 * each component must only render/re-render only when their state is UI_STATE_INIT, redraw command (2016.03.05)
 * to increase portability between LCD device, driver APIs must be (can be) provided during runtime, see gui_handle documentation (2016.03.05)
 * added some if_config_write and if_config_read to read/store NVRAM (2016.03.15)
 * added gui_handle with put_pixel and begin_burst for rendering icon image (2016.03.16)
 * added support to load icon from 2 bit file (*.OCI) (2016.03.16)
 * changed touch state parameter/configuration to g_uconfig (added struct ui_config) which can be save/restore from NVRAM (2016.03.17)
 * added : ui_alert for displaying information during processing (2016.03.XX)
 * added : new component ui_combo (2016.03.31)
 * modified: ui_panel_create added border and radius parameter (2016.03.31)
 * modified: ui_text and ui_create_text, removing text from ui_object, reducing memory footprint when allocating ui_object (2016.04.01)
 * modified: ui_image_render render image at the middle of ui_rect (2017.05.02)
 * added: ui_draw_button for multipurpose button rendering, use ui_draw_button for rendering keyboard button (2017.05.17)
 * fixed: ui_keyboard shift_in, shift_out button rendering on multitasking environment (2017.05.17) 
 * fixed: ui_image_render to use gba_net_buffer in-case no heap available (2017.07.28)
 * changed: ui_button_render to load image from PNG during button rendering (2017.09.03)
 * fixed: ui_orbriver_item_render modifying oid data (2017.09.11)
1.1
 * added: ui_get_screen_by_name search specified screen from display stack (2018.05.09)
 * changed: ui_object, added rect property to store original ui_rect for relative position (2018.05.22)
 * added: view_rect on gui_handle, contain configuration for viewing rect (2018.08.03)
1.2 
 * added: animation core (2018.08.09)
 * changed : modified ui_list_view (2018.10.07)
 * renamed : ui_panel_create to ui_create_panel, added UI_TYPE_PANEL (2018.10.08)
 * added: auto-resize layout some components (ui_icon_tool_render, etc) (2018.10.15)
********************************************************************
\endverbatim
 */
#ifndef UI_CORE__H
#define UI_CORE__H
#include "defs.h"
#include "config.h"
#include <stdlib.h>
#include "..\..\core\inc\os_core.h"

#define UI_MAX_STACK				10
#define UI_MAX_TEXT				128
#define UI_MAX_OBJECT_NAME		16
#define UI_MAX_SSID_NAME			40
#define UI_MAX_SSID_PASSWORD	28
#define UI_OBJECT_MAGIC				0xCC

#define UI_PXFMT_332			0x332
#define UI_PXFMT_444			0x444
#define UI_PXFMT_555			0x555
#define UI_PXFMT_565			0x565
#define UI_PXFMT_666			0x666
#define UI_PXFMT_888			0x888

//ui constant definition
#define UI_COLOR_RGB(r,g,b)		(((r<<16) | (g<<8) | (b)) | 0xFF000000)
#define UI_COLOR_WHITE			0xFFFFFFFF
#define UI_COLOR_BLACK			0xFF000000
#define UI_COLOR_RED				0xFFFF0000
#define UI_COLOR_GREEN			0xFF00FF00
#define UI_COLOR_BLUE			0xFF0000FF
#define UI_COLOR_QUEEN_BLUE		0xFF436B95
#define UI_COLOR_SLATE_GREY		0xFF708090
#define UI_COLOR_LIGHT_GREY		0xFFCCCCCC
#define UI_COLOR_GREY			0xFF888888
#define UI_COLOR_DARK_GREY	0xFF444444
#define UI_COLOR_ORANGE		0xFFFF6600
#define UI_COLOR_LIME				0xFF00FF00
#define UI_COLOR_GOLD			0xFFFFD700

#define UI_TYPE_LABEL					0x00010000
#define UI_TYPE_BUTTON					0x00020000
#define UI_TYPE_GRIDITEM				0x00040000
#define UI_TYPE_PANEL					0x00080000
#define UI_TYPE_CONTAINER				0x00800000
#define UI_TYPE_VISIBLE					0x01000000
#define UI_TYPE_SELECTABLE				0x80000000
#define UI_TYPE_TEXT					0x00008000

#define UI_ALIGN_NONE				0x00000000
#define UI_ALIGN_GRID					0x00000200
#define UI_ALIGN_VERTICAL			0x00000800
#define UI_ALIGN_HORIZONTAL		0x00000400
#define UI_ALIGN_FULL					0x00000C00
#define UI_ALIGN_FLOAT				0x00000F00
#define UI_ALIGN_MASK					0x00000F00

#define UI_FONT_DIGITAL				29
#define UI_FONT_DEFAULT				16
#define UI_FONT_DEFAULT_WIDTH		8
#define UI_FONT_SMALL				8
#define UI_FONT_SMALL_WIDTH			6

#define UI_LCS_AVAILABLE				0x00
#define UI_LCS_DETACHED				0x40

#define UI_STATE_KEYDOWN			0x0800
#define UI_STATE_KEYUP				0x1000
#define UI_STATE_INIT					0x8000

#define UI_STATE_ACTIVE				0x0002
#define UI_STATE_FOCUS				0x0001
#define UI_STATE_SELECTED			0x0010
#define UI_STATE_DESELECTED		0x0020

#define UI_KEY_DOWN		0x0001
#define UI_KEY_UP			0x0000

#define UI_DISP_ORIENTATION_0		0x0
#define UI_DISP_ORIENTATION_1		0x1
#define UI_DISP_ORIENTATION_2		0x2
#define UI_DISP_ORIENTATION_3		0x3

#define UI_TOGGLE_DISABLED		0x80
#define UI_TOGGLE_ON					1
#define UI_TOGGLE_OFF				0

#define UI_RGB565					0x565
#define UI_RGB888					0x888
#define UI_RGB332					0x332
#define UI_ARGB8888				0x8888

#define UI_IMAGE_ALIGN_TOP				0x00
#define UI_IMAGE_ALIGN_LEFT				0x00
#define UI_IMAGE_ALIGN_BOTTOM		0x01
#define UI_IMAGE_ALIGN_RIGHT			0x02
#define UI_IMAGE_ALIGN_CENTER		0x04
#define UI_IMAGE_ALIGN_FILL				0x10

#define UI_ANIM_NONE				0x0000
#define UI_ANIM_ALPHA_BLENDING		0x0100
#define UI_ANIM_SLIDE_LEFT			0x0200
#define UI_ANIM_SLIDE_RIGHT			0x0400

#define IF_COLOR(x) ( ((x >> 8) & 0xF800) | ((x >> 5) & 0x07E0) | ((x>>3) & 0x001F))
#define IF_COLOR_RGB(r,g,b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b>>3) & 0x001F))
#define IF_COLOR_RGB888(r,g,b) ( ((uint32)r<< 16) | ((uint32)g<< 8) | (b & 0x00FF))
#define IF_COLOR_ARGB8888(r,g,b) ( 0xFF000000 | ((uint32)r << 16) | ((uint32)g<< 8) | (b & 0x00FF))

#define UI_STATUS_WAIT						0x01			//wait response state
#define UI_STATUS_ACTIVE					0x80			//active display screen

//ui struct definition
typedef struct ui_rect ui_rect;
typedef struct ui_size ui_size;
typedef struct ui_buffer ui_buffer;
typedef struct ui_resource ui_resource;
typedef struct ui_image ui_image;
//typedef struct ui_view ui_view;
typedef struct ui_object ui_object;
typedef struct ui_text ui_text;
typedef struct ui_toggle ui_toggle;
typedef struct ui_container ui_container;
typedef struct ui_app_object ui_app_object;
typedef struct ui_item_object ui_item_object;

//ui typedef definition
typedef struct gui_handle gui_handle;
typedef struct gui_handle * gui_handle_p;

typedef struct ui_config ui_config;
typedef struct ui_config * ui_config_p;

typedef struct ui_config {
	uint8 magic;
//touch configuration
	uint16 x;
	uint16 y;						   	    
	uint8  t_state;
	float xfac;
	float yfac;
	int16 xoff;
	int16 yoff;
//wifi configuration
	//uint8 ssid_name[UI_MAX_SSID_NAME];
	//uint8 ssid_password[UI_MAX_SSID_PASSWORD];
} ui_config;

extern ui_config g_uconfig;

struct ui_rect {
	uint16 x;
	uint16 y;
	uint16 w;
	uint16 h;
};

struct ui_size {
	uint16 w;
	uint16 h;
};

struct ui_buffer {
	ui_size size;
	uint16 pxfmt;
	uint32 length;
	uint8 * buffer;
};

struct ui_image {
	ui_buffer base;
	uint8 data[0];
};

#if 0
struct ui_view {
	ui_rect base;
	uint8 mleft;				//margin left
	uint8 mtop;				//margin top
	uint8 mright;				//margin right
	uint8 mbottom;			//margin bottom
	uint8 pleft;					//padding left
	uint8 ptop;					//padding top
	uint8 pright;				//padding right
	uint8 pbottom;			//padding bottom
};
#endif

struct ui_resource {
	uint16 size;
	uint8 * buffer;
};

struct ui_object {
	struct ui_rect base;
	struct ui_rect rect;
	struct ui_object * parent;
	struct ui_object * sibling;
	struct ui_object * child;
	void (* release)(void *);
	void * display;				//current object display handle
	uint8 magic;
	uint8 lcs;
	//uint8 align;				//0 = left top, 1 = right top, 2 = left bottom, 3 = right bottom
	uint32 type;
	uint16 state;
	uint32 backcolor;
	uint32 forecolor;
	//image
	void (* img_release)(void * image);
	ui_resource image;
	uint8 name[UI_MAX_OBJECT_NAME];			//internal name
	void (* render)(struct ui_object * object, gui_handle_p);
#if SHARD_ENABLE_PRERENDERED
	void (* prerender)(struct ui_object * object, gui_handle_p);
	ui_buffer * prerendered;
#endif
	void (* handler)(struct ui_object *, void * params);
	void * target;			//pointer to tagged object
	//animation core
	uint16  ac_state;		//animation state
	uint16  ac_duration;		//animation duration (fps)
	ui_rect ac_irect;			//initial rect
	ui_image * ac_foreground;			//animation core image
	ui_image * ac_background;			//animation core image
};

struct ui_text {
	ui_object base;
	char text[UI_MAX_TEXT];
};

struct ui_toggle {
	ui_text base;
	uint8 state;
};

struct ui_container {
	ui_object base;
	ui_object * selected;
};

struct gui_handle {
#if HWCL_ENABLED
	DMA2D_HandleTypeDef base;
#endif
	uint16 width;
	uint16 height;
	uint8 fps;							//current display FPS
	ui_config * touch_config;
	struct ui_object * header;
	struct ui_object * meta;
	struct ui_object * body;
	ui_rect view_rect;					//contain configuration for viewing rect (bodies), added 2018.08.03
	uint8 status;
	uint8 brightness;	
	uint8 orientation;
	ui_object * stack[UI_MAX_STACK];
	uint8 stack_index;
	//framebuffer context
	uint16 fb_pxfmt;				//pixel format
	uint32 fb_size;					//sizeof framebuffer
	void * fb_ptr;					//display framebuffer
	uint32 ac_support;			//available animation core
	//display driver APIs			
	uint8 (* init)(struct gui_handle *);
	void (* wake)(gui_handle_p display);
	void (* sleep)(gui_handle_p display);
	void (* switch_orientation)(gui_handle_p display, uint8 mode);
	void (* set_backlight)(struct gui_handle *, uint8 mode);
	//driver drawing APIs
	uint32 (* set_area)(struct gui_handle *, uint16 x, uint16 y, uint16 w, uint16 h);
	void (* begin_burst)(struct gui_handle *);					//prepare for write
	void (* put_pixel)(struct gui_handle *, uint32 color);		//write
	void (* skip_pixel)(struct gui_handle *);					//skip put pixel for specified area
	void (* present)(struct gui_handle *);						//start presenting framebuffer
	//generic drawing APIs
	uint16 (* print_string)(struct gui_handle *, uint8 size, uint16 x, uint16 y, uint8 * str, uint32 color);
	uint16 (* measure_string)(struct gui_handle *, uint8 size, uint8 * str);
	void (* draw_line)(struct gui_handle *, uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint32 color);
	void (* draw_rectangle)(struct gui_handle *, uint16 x, uint16 y, uint16 w, uint16 h, uint32 color, uint8 radius);
	void (* fill_area)(struct gui_handle *, uint32 color, uint32 size);
	void (* fill_bitmap)(struct gui_handle *, uint32 forecolor, uint32 backcolor, uint8 * map, uint32 size);
	
	//touch APIs
	uint8 (* touch_state)(struct gui_handle *);
	uint8 (* touch_read)(struct gui_handle *, uint16 * x, uint16 * y);
	uint8 touch_treshold[6];
	uint16 touch_status;
};

void ui_display_init(gui_handle_p handle);
void ui_net_init(gui_handle_p handle, void * netctx);
void ui_card_init(gui_handle_p handle, void * cardctx);
void * ui_create_object(size_t objsize, uint32 mode) ;
void * ui_create_text(size_t objsize, uint32 mode, char * text);
uint8 ui_add_object(ui_object * parent, ui_object * obj);
void ui_set_toggle_state(ui_object * object, uint8 state);
void ui_set_target(ui_object * obj, void * target) ;
void ui_set_align(ui_object * obj, uint32 align);
void ui_set_size(ui_object * obj, uint16 w, uint16 h);
void ui_set_position(ui_object * obj, uint16 x, uint16 y);
void ui_set_text(ui_text * obj, uint8 * text);
void ui_set_image(ui_object * obj, uint8 * image, uint16 img_size);
uint8 ui_add_header(gui_handle_p display, void * object);
uint8 ui_add_body(gui_handle_p display, void * object) ;
void ui_clear_body(gui_handle_p display, ui_object * screen);
void ui_clear_child(gui_handle_p display, ui_object * parent);
void ui_clear_dispstack(gui_handle_p display);
void ui_switch_orientation(gui_handle_p display, uint8 orientation);
void ui_reinitialize_object(ui_object * obj) ;
void ui_set_object_name(ui_object * obj, const char * name);
ui_object * ui_get_object_by_name(gui_handle_p display, const char * name) ;
ui_object * ui_get_object_by_xy(gui_handle_p display, uint16 x, uint16 y); 
void ui_detach_object(ui_object * obj) ;
void ui_delete_object(ui_object * obj);
void ui_invalidate_area(ui_object * root, ui_rect * rect);
void ui_set_selected_object(gui_handle_p display, ui_object * obj);
void ui_render(gui_handle_p display, ui_object * iterator);
ui_object * ui_process_events(gui_handle_p display);
void * ui_present(gui_handle_p display);
ui_object * ui_wait_user_input(gui_handle_p display) ;
void ui_clear_selected(gui_handle_p display);
ui_object * ui_push_screen(gui_handle_p display, ui_object * panel);
ui_object * ui_pop_screen(gui_handle_p display) ;
ui_object * ui_pop_screen_unsafe(gui_handle_p display);
ui_object * ui_get_screen_by_name(gui_handle_p display, const char * name);
ui_object * ui_remove_screen_unsafe(gui_handle_p display, ui_object * screen);
void ui_remove_screen(gui_handle_p display, ui_object * screen);
uint8 ui_is_screen_exist(gui_handle_p display, ui_object * screen);

//internal renderer
uint8 ui_check_overlapped(ui_rect * a, ui_rect * b);
uint8 ui_check_intersect(ui_rect * a, uint16 x, uint16 y);
void ui_rect_copy(ui_buffer * src, ui_rect * src_rect, ui_buffer * dst, ui_rect * dst_rect, uint16 pxfmt) ;
ui_image * ui_image_alloc(ui_size * sz, uint16 pxfmt);
uint8 ui_image_info(ui_resource * image, uint16 pxfmt, uint16 * w, uint16 * h) ;
void ui_image_render(gui_handle_p display, ui_rect * obj, uint8 * bitmap, uint16 bmpsize, uint8 align);
void ui_resource_render(gui_handle_p display, ui_rect * obj, ui_resource * image, uint8 align);
void ui_fill_image(gui_handle_p handle, ui_rect * rect, ui_image * image);
void ui_tab_render(ui_object * obj, gui_handle_p display);

//animation core APIs
uint8 ui_get_pixel_size(uint16 pxfmt);
void ui_fb_setup_animation(ui_object * obj);
void ui_set_animation(ui_object * obj, uint16 mode);
void ui_fb_copy(gui_handle_p handle, ui_buffer * dst, ui_rect * rect);
void ui_fb_copy_resize_image(gui_handle_p handle, ui_buffer * dst, ui_rect * rect);
ui_image * ui_fb_crop_image(gui_handle_p handle, ui_rect * rect);
ui_image * ui_fb_resize_image(gui_handle_p handle, ui_rect * rect, ui_size * size);
uint8 ui_fb_animate(gui_handle_p handle, ui_object * obj);

#endif
