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

extern const unsigned char g_dxvb_appicon_png_48[3526];			//app icon

static void dashboard_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(dashboard_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint32 longtime = 0;
	ui_object * screen;
	ui_object * cfBtn;
	//tk_clear_subconfig_menu(ctx);
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "dxvb"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "dashboardApp") ;
	if(cfBtn != NULL) {
		((ui_icon_tool *)cfBtn)->show_marker = FALSE;
	}
	//ui_clear_dispstack(ctx->display);
	//tk_clear_body(ctx);
	OS_DEBUG_EXIT();
}


extern void power_vending_app_init(gui_handle_p handle, void * params);
extern void calculator_app_init(gui_handle_p handle, void * params) ;
extern void chart_app_init(gui_handle_p handle, void * params);
extern void spectral_app_init(gui_handle_p handle, void * params);
extern void file_manager_app_init(gui_handle_p handle, void * params) ;
extern void vnes_app_init(gui_handle_p handle, void * params) ;
extern void microphone_app_init(gui_handle_p handle, void * params, uint8 id);


static void dashboard_show(tk_context_p ctx) {
	OS_DEBUG_ENTRY(dashboard_show);
	ui_toggle * play;
	ui_datetime * dtime;
	datetime dval;
	ui_object * list;
	uint32 longtime;
	ui_object * obj;
	ui_object * calc_keypad1, * calc_keypad2;
	uint16 keypad_height = 0;
	uint16 y = 0;
	tk_config * conf = ctx->config;
	ui_object * lcd;
	ui_object * chart;
	ui_series * series;
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	//ui_clear_dispstack(ctx->display);
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	obj = ui_get_object_by_name(ctx->display, "dashboardApp") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "dxvb") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "dxvb");
	ui_set_text((ui_text *)screen, (uint8 *)"dxvb");
	
	power_vending_app_init(ctx->display, ctx);
	calculator_app_init(ctx->display, ctx);
	chart_app_init(ctx->display, ctx);
	spectral_app_init(ctx->display, ctx);
	file_manager_app_init(ctx->display, ctx);
	vnes_app_init(ctx->display, ctx);
	microphone_app_init(ctx->display, ctx, 0);
	
	//series = ui_series_create_bar("test", sizeof(uint16), UI_COLOR_BLUE, UI_COLOR_LIME, sizeof(sample_bar_series) / sizeof(uint16), (void *)sample_bar_series);
	//ui_add_body(ctx->display, (chart = ui_chart_create("chart", series)));
	//ui_set_align(chart, UI_ALIGN_FULL);
	
	//ui_set_size(chart, screen->base.w, screen->base.h);
	//ui_add_body(ctx->display, (lcd = ui_numbox_create((uint8 *)"", 40)));
	//ui_set_object_name(lcd, "calc_lcd");
	//ui_set_content(lcd, "0");
	//y = screen->rect.y;
	//y += lcd->rect.h + 8;
	//keypad_height = screen->rect.h -  (lcd->rect.h + 8);
	
	if(((gui_handle_p)ctx->display)->orientation & 0x01) {
		//ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w, keypad_height / 2, 0)));
		//ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x, y + (keypad_height / 2), screen->rect.w, keypad_height / 2, 1)));
	} else {
		//ui_add_body(ctx->display, (calc_keypad1 = ui_calcpad_create(lcd, screen->rect.x, y, screen->rect.w/2, keypad_height, 0)));
		//ui_add_body(ctx->display, (calc_keypad2 = ui_calcpad_create(lcd, screen->rect.x + (screen->rect.w/2), y, screen->rect.w/2, keypad_height, 1)));
	}
	
	OS_DEBUG_EXIT();
}

static void dashboard_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(dashboard_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * instance;
	ui_textbox * textbox;
	uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "dxvb")) != NULL) {
		if(((gui_handle_p)ctx->display)->body == screen) {
			dashboard_exit_click(obj, params);
		} else {
			//tk_clear_subconfig_menu(ctx);
			ui_remove_screen_unsafe(ctx->display, screen);
			screen = ui_push_screen(ctx->display, screen);
			ui_set_object_name(screen, "dxvb");
			ui_set_text((ui_text *)screen, (uint8 *)"dxvb");
			ui_set_target(screen, ctx);
		}
	} else {
			dashboard_show(ctx);
	}
	OS_DEBUG_EXIT();
	return;
}


void dashboard_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(tk_setting_init);
	ui_object * calc = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (calc = ui_header_create((uint8 *)"dashboardApp", (uint8 *)"dxvb", header_w, header_w, sizeof(g_dxvb_appicon_png_48), (uint8 *)g_dxvb_appicon_png_48, dashboard_click)));
		if(calc != NULL) ui_set_target(calc, params);
	}
	OS_DEBUG_EXIT();
}