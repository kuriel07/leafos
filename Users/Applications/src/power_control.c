#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\build.h"
#include "..\..\config.h"
#include "..\..\gui\inc\ui_resources.h"
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

extern uint8_c pwr_vending_appicon_png_48[2415];		//from power_vending_resources.h

static void pvending_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(pvending_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint32 longtime = 0;
	ui_object * screen;
	ui_object * cfBtn;
	//tk_clear_subconfig_menu(ctx);
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "power"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "myVending") ;
	if(cfBtn != NULL) {
		((ui_icon_tool *)cfBtn)->show_marker = FALSE;
	}
	//ui_clear_dispstack(ctx->display);
	//tk_clear_body(ctx);
	OS_DEBUG_EXIT();
}


static void pvending_show(tk_context_p ctx) {
	OS_DEBUG_ENTRY(pvending_show);
	ui_toggle * play;
	ui_datetime * dtime;
	datetime dval;
	ui_object * list;
	uint32 longtime;
	ui_object * obj;
	tk_config * conf = ctx->config;
	ui_object * lcd;
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	//ui_clear_dispstack(ctx->display);
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	obj = ui_get_object_by_name(ctx->display, "myVending") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "power") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "power");
	ui_set_text((ui_text *)screen, (uint8 *)"power");
	ui_add_body(ctx->display, (lcd = ui_numbox_create((uint8 *)"", 40)));
	ui_set_content(lcd, "1234567890");
	
	OS_DEBUG_EXIT();
}

static void pvending_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(pvending_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * instance;
	ui_textbox * textbox;
	uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "power")) != NULL) {
		if(((gui_handle_p)ctx->display)->body != screen) {
			pvending_exit_click(obj, params);
		} else {
			//ui_remove_screen_unsafe(ctx->display, screen);
			//screen = ui_push_screen(ctx->display, screen);
			//ui_set_object_name(screen, "pvending");
			//ui_set_text((ui_text *)screen, (uint8 *)"pvending");
		}
	} else {
			pvending_show(ctx);
	}
	OS_DEBUG_EXIT();
	return;
}


void power_vending_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(tk_setting_init);
	ui_object * vending = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (vending = ui_header_create((uint8 *)"myVending", (uint8 *)"power", header_w, header_w, sizeof(pwr_vending_appicon_png_48), (uint8 *)pwr_vending_appicon_png_48, pvending_click)));
		if(vending != NULL) ui_set_target(vending, params);
	}
	OS_DEBUG_EXIT();
}


void power_vending_app_init(gui_handle_p handle, void * params, uint8 id) {
	OS_DEBUG_ENTRY(power_vending_app_init);
	ui_object * vending = NULL;
	ui_object * body = handle->body;
	ui_object * obj;
	uint16 header_w = 64;
	if(params != NULL && body != NULL) {
		ui_add_body(handle, (vending = ui_launcher_create((uint8 *)"myVending", (uint8 *)"power", header_w, header_w, sizeof(pwr_vending_appicon_png_48), (uint8 *)pwr_vending_appicon_png_48, pvending_click)));
		if(vending != NULL) {
			obj = ui_get_object_by_name(handle, "power") ;
			if(obj != NULL) {
				((ui_icon_tool *)vending)->show_marker = TRUE;
			}
			ui_set_target(vending, params);
		}
	}
	OS_DEBUG_EXIT();
}