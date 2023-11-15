#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "..\inc\ui_window.h"
#include "..\inc\ui_panel.h"
#include "..\inc\ui_label.h"
#include "..\inc\ui_scroll.h"
#include "..\inc\ui_resources.h"

static void ui_window_render(ui_object * obj, gui_handle_p display) {
	uint16 xx, yy, nc, wtxt;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 dw;
	uint16 xdw;
	uint16 i,j,k;
	uint8 temp[32];
	ui_object mold;
	ui_object icon_rect;
	uint16 last_space;
	xx = 4;		//x = width - text_width / 2;
	yy = ((24 - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
	if(obj->state & UI_STATE_INIT) {
		//display->draw_rectangle(display, x, y, w, h, UI_COLOR_BLACK, 1);
		//display background first
		display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		display->draw_rectangle(display, x, y, w-1, h-1, UI_COLOR_GREY, 0);
		display->set_area(display, x, y , w, h);
		nc = strlen((const char *)((ui_text *)obj)->text);
		wtxt = nc * 8;
		xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
		yy = 4;
		//print title
		
		memcpy(&mold, obj, sizeof(ui_object));
		((ui_rect *)&mold)->h = 24;
		ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_window_bar24, sizeof(image_png_window_bar24), UI_IMAGE_ALIGN_FILL);
		display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)((ui_text *)obj)->text, UI_COLOR_WHITE);		//title
		//display->draw_rectangle(display, x + 10, y + UI_FONT_DEFAULT, w - 20, 0,  UI_COLOR_GREY, 0);
		display->set_area(display, x, y , w, h);
		xx = 4;		//x = width - text_width / 2;
		yy += 28;
	}
	//display->print_string(display, UI_FONT_DEFAULT, x, (y + h) - UI_FONT_DEFAULT, ((ui_object *)obj)->text, ((ui_object *)obj)->forecolor);
}

ui_window * ui_window_show(gui_handle_p display, uint8 * text, uint16 mode, uint8 icon) {
	uint8 i;
	ui_object * list;
	uint16 wdiv;
	uint16 xdiv = 0;
	uint16 w = display->width / 2;
	uint16 h = w;
	uint16 y = (display->height - (w  + 24)) / 2;
	uint16 height = 24 * 5;
	uint16 x = w / 2;
	ui_window * obj = (ui_window *)ui_create_panel(sizeof(ui_window), x, y, w, h, text, 0, 0) ;
	((ui_object *)obj)->rect.w = display->width / 2;
	((ui_object *)obj)->rect.h = height;
	((ui_object *)obj)->rect.x = ((ui_object *)obj)->rect.w / 2;
	((ui_object *)obj)->rect.y = (((display->height - 24) - (height  + 24)) / 2) + 24;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(61,63,71);
	((ui_object *)obj)->render = ui_window_render;
	((ui_object *)obj)->handler = NULL;
	obj->mode = mode;
	obj->icdx = icon;
	ui_add_object((ui_object *)obj, (list = ui_list_create(1, 24, ((ui_object *)obj)->rect.w - 2, ((ui_object *)obj)->rect.h - 26)));
	ui_set_align(list, UI_ALIGN_NONE);
	//change display status to wait mode
	obj->panel = (ui_object *)((ui_list_view *)list);
	display->status |= UI_STATUS_WAIT;
	ui_push_screen(display, (ui_object *)obj);
#if SHARD_RTOS_ENABLED == 0
	ui_present(display);
#else 
	os_wait(50);		//wait till new screen rendered	(2017.07.06)
#endif
	return obj;
}

void ui_window_close(gui_handle_p display, ui_window * window) {
	//ui_pop_screen(display);
	ui_remove_screen(display, (ui_object *)window);
	//change display status to normal operation mode
	display->status &= ~UI_STATUS_WAIT;
#if SHARD_RTOS_ENABLED == 0
	ui_present(display);
#else
	os_wait(40);
#endif
}











