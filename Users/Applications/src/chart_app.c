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

extern const unsigned char g_chart_appicon_png_48[1980];			//app icon
extern const unsigned char g_chart_appicon_png_32[1430];

static void chart_exit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(chart_exit_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	uint32 longtime = 0;
	ui_object * screen;
	ui_object * cfBtn;
	//tk_clear_subconfig_menu(ctx);
	ui_remove_screen(ctx->display, ui_get_screen_by_name(ctx->display, "cpu_util"));
	//disable marker
	cfBtn = ui_get_object_by_name(ctx->display, "chartApp") ;
	if(cfBtn != NULL) {
		((ui_icon_tool *)cfBtn)->show_marker = FALSE;
	}
	//ui_clear_dispstack(ctx->display);
	//tk_clear_body(ctx);
	OS_DEBUG_EXIT();
}

const uint16 sample_bar_series[] = { 32, 150, 75, 60, 99, 144, 8, 69, 192, 188, 96, 60, 40 };

//performance buffer as global variable to be shared between windows
uint16 g_cpu_perf_buffer[48];
//uint16 g_audio_buffer[AUDIO_SAMPLING_FREQ];

static void perf_task() {
	OS_DEBUG_ENTRY(perf_task);
	tk_context_p ctx;
	ui_object * obj;
	uint8 orientation = 0;
	uint16 i, j=0;
	ui_chart * chart = NULL;
	ui_series * cpu_perf = NULL;
	ui_infostat * infostat;
	ctx = os_get_context();
	
	while(1) {
		os_wait(200);		//tick at 1 second
		chart = (ui_chart *)ui_get_object_by_name(ctx->display, "perf_chart");
		if(chart != NULL) {
			cpu_perf = ui_chart_get_series_by_name(chart, "cpu_perf");
			if(cpu_perf != NULL) {
				//shift elements to left
				memcpy(g_cpu_perf_buffer, g_cpu_perf_buffer + 1, sizeof(g_cpu_perf_buffer) - sizeof(uint16));
				//tk_memcpy(g_cpu_perf_buffer, g_cpu_perf_buffer + 1, sizeof(g_cpu_perf_buffer) - sizeof(uint16));
				infostat = (ui_infostat *)ui_get_object_by_name(ctx->display, "infoBar");			//get meta object (infobar)
				if(infostat != NULL) {
					g_cpu_perf_buffer[48 -1] = infostat->cpu_util;
				}
				//test audio sampling
				//ctx->audio->read(ctx->audio, g_audio_buffer, sizeof(g_audio_buffer));
				//for(i=0,j=0;i<AUDIO_SAMPLING_FREQ;i+=(AUDIO_SAMPLING_FREQ/48),j++) {
				//	g_cpu_perf_buffer[j] = ((int16)g_audio_buffer[i]) + 256;
				//}
				memcpy(cpu_perf->elements, g_cpu_perf_buffer, sizeof(g_cpu_perf_buffer));
				ui_reinitialize_object(chart);				//invalidate chart
			}
		}
	}
	
	OS_DEBUG_EXIT();
}

static void chart_show(tk_context_p ctx) {
	OS_DEBUG_ENTRY(chart_show);
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
	ui_chart * chart;
	ui_series * series;
	os_task * cpuperf = NULL;
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	//ui_clear_dispstack(ctx->display);
	//ui_clear_body(ctx->display, UI_COLOR_WHITE);
	obj = ui_get_object_by_name(ctx->display, "chartApp") ;
	if(obj != NULL) {
		((ui_icon_tool *)obj)->show_marker = TRUE;
	}
	obj = ui_get_object_by_name(ctx->display, "cpu_util") ;
	if(obj != NULL) {
		ui_remove_screen(ctx->display, obj);
	}
	ui_object * screen = ui_push_screen(ctx->display, NULL);
	ui_set_object_name(screen, "cpu_util");
	ui_set_text((ui_text *)screen, (uint8 *)"cpu_util");
	
	series = ui_series_create_line("cpu_perf", sizeof(uint16), UI_COLOR_BLUE, UI_COLOR_LIME, sizeof(g_cpu_perf_buffer) / sizeof(uint16), (void *)g_cpu_perf_buffer);
	ui_add_body(ctx->display, (chart = (ui_chart *)ui_chart_create("perf_chart", "Utilization", series)));
	chart->area.axis[1].max = 100;			//max percentage for cpu util
	//((ui_chart *)chart)->area.mode = UI_CHART_MODE_RELATIVE;
	ui_set_align((ui_object *)chart, UI_ALIGN_FULL);
	
	cpuperf = os_find_task_by_name("cpuperf");
	if(cpuperf == NULL) {
		memset(g_cpu_perf_buffer, 0, sizeof(g_cpu_perf_buffer));		//clear performance buffer
		cpuperf = os_create_task(ctx, perf_task, "cpuperf", 500, 2048);
	}
	
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

static void chart_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(chart_click);
	tk_context_p ctx = (tk_context_p)obj->target;
	ui_object * instance;
	ui_textbox * textbox;
	uint16 ydiv = 0;
	uint8 tbuf[256];
	ui_object * screen;
	memset(tbuf, 0, sizeof(tbuf));
	if((screen = ui_get_screen_by_name(ctx->display, "cpu_util")) != NULL) {
		if(((gui_handle_p)ctx->display)->body != screen) {
			chart_exit_click(obj, params);
		} else {
			//ui_remove_screen_unsafe(ctx->display, screen);
			//screen = ui_push_screen(ctx->display, screen);
			//ui_set_object_name(screen, "chart");
			//ui_set_text((ui_text *)screen, (uint8 *)"chart");
		}
	} else {
			chart_show(ctx);
	}
	OS_DEBUG_EXIT();
	return;
}


void chart_init(gui_handle_p handle, void * params) {
	OS_DEBUG_ENTRY(chart_init);
	ui_object * calc = NULL;
	ui_object * header = handle->header;
	uint16 header_w;
	if(params != NULL && header != NULL) {
		header_w = header->rect.h / 4;
		ui_add_header(handle, (calc = ui_header_create((uint8 *)"chartApp", (uint8 *)"cpu_util", header_w, header_w, sizeof(g_chart_appicon_png_32), (uint8 *)g_chart_appicon_png_32, chart_click)));
		if(calc != NULL) ui_set_target(calc, params);
	}
	OS_DEBUG_EXIT();
}

void cpu_util_appicon_render(ui_object * obj, gui_handle_p display) {
	uint16 nc, wtxt, xx, yy;
	uint8 ww, hh, num_pal;
	uint8 i, j, c;
	//uint32 palletes[8];
	uint16 offset;
	uint32 color;
	uint8 index;
	ui_infostat * infostat;
	static double cpu_util = 0;
	char buffer[16];
	//ui_object frame;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 font_size = UI_FONT_SMALL;
	uint8 font_width = UI_FONT_SMALL_WIDTH;
	ui_rect bkg_rect = { x, y, w, h - 16 };
	ui_icon_bitmap * objmp = &((ui_icon_tool *)obj)->bitmap[((ui_toggle *)obj)->state & 0x01];
	if(((ui_rect *)obj)->h <= 40) {
		font_size = UI_FONT_SMALL;
		font_width = UI_FONT_SMALL_WIDTH;
	}
	//if(obj->state != 0) {
		//ui_tab_render(obj, display);
		//display->set_area(display, x, y, w, h);
		display->fill_area(display, obj->backcolor, display->set_area(display, x, y, w, h));
		if(objmp != NULL) {
			ui_image_render(display, (ui_rect *)&bkg_rect, objmp->bitmap, objmp->bmpsize, UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
			if(((ui_rect *)obj)->h <= 40) goto exit_icon_tool_render;
		}
		infostat = (ui_infostat *)ui_get_object_by_name(display, "infoBar");			//get meta object (infobar)
		if(infostat != NULL) {
			//g_cpu_perf_buffer[48 -1] = infostat->cpu_util;
			cpu_util += infostat->cpu_util;
			cpu_util /= 2;
			snprintf(buffer, sizeof(buffer), "%0.1f%%", cpu_util);
			nc = strlen((const char *)(buffer));
			wtxt = nc * font_width;
			xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
			display->print_string(display, font_size, x + xx, (y + h) - (2 * font_size), (uint8 *)buffer, UI_COLOR_WHITE);
			//snprintf(buffer, sizeof(buffer), "cpu:%0.1f%%", infostat->);
			//display->print_string(display, font_size, x + 4, y + 36, (uint8 *)buffer, UI_COLOR_WHITE);
		}
		//display->print_string(display, UI_FONT_DEFAULT, x + xx, (y + h) - UI_FONT_DEFAULT, ((ui_object *)obj)->text, ((ui_object *)obj)->forecolor);
		//if(((ui_rect *)obj)->h <= 40 && objmp != NULL) {
			//don't print text
		//} else {
		nc = strlen((const char *)((ui_text *)obj)->text);
		wtxt = nc * font_width;
		xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
		display->set_area(display, x, y, w, h);
		display->print_string(display, font_size, x + xx, (y + h) - font_size, (uint8 *)((ui_text *)obj)->text, UI_COLOR_WHITE);
		//}
		//display->fill_area(display, UI_COLOR_RGB(79,81,89), display->set_area(display, x, y + h-2, w, 1));
		//display->fill_area(display, UI_COLOR_RGB(60,62,70), display->set_area(display, x, y + h-1, w, 1));
	//} 
	exit_icon_tool_render:
	ui_launcher_draw_marker(obj, display);
}

void chart_app_init(gui_handle_p handle, void * params, uint8 id) {
	OS_DEBUG_ENTRY(chart_app_init);
	ui_object * app_icon = NULL;
	ui_object * body = handle->body;
	ui_object * obj;
	uint16 header_w = 64;
	if(params != NULL && body != NULL) {
		ui_add_body(handle, (app_icon = ui_launcher_create((uint8 *)"chartApp", (uint8 *)"cpu_util", header_w, header_w, sizeof(g_chart_appicon_png_32), (uint8 *)g_chart_appicon_png_32, chart_click)));
		if(app_icon != NULL) {
			obj = ui_get_object_by_name(handle, "cpu_util") ;
			if(obj != NULL) {
				((ui_icon_tool *)app_icon)->show_marker = TRUE;
			}
			ui_set_target(app_icon, params);
		}
		app_icon->render = cpu_util_appicon_render;
	}
	OS_DEBUG_EXIT();
}