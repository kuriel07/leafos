
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_gauge.h"
#include "..\inc\ui_core.h"
#include "..\inc\ui_slider.h"
#include "..\..\interfaces\inc\if_apis.h"

static void ui_slider_render(ui_object * obj, gui_handle_p display) {
	uint16 xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 i;
	uint16 j;
	uint16 xlen, x1, wtxt, nc;
	uint8 linebuf[27];
	ui_gauge * gauge = (ui_gauge *)obj;
	if(obj->state & UI_STATE_INIT) {
		switch(gauge->orientation) {
			case UI_GAUGE_HORIZONTAL:
				//draw border
				display->draw_rectangle(display, x+8, y+8, w-16, 16, UI_COLOR_WHITE, 1);
				display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, x + 9, y + 9, w - (16+2), 14));
				xlen = w - (16+2);
				x1 = (((ui_gauge *)obj)->percent * xlen) / 100;
				//display->fill_area(display, UI_COLOR_GREY, display->set_area(display, x+3, y+3, xlen, h-6));
				display->fill_area(display, UI_COLOR_SLATE_GREY, display->set_area(display, x + 9, y + 9, x1, 14));
				break;
			case UI_GAUGE_VERTICAL:
				//draw border
				display->draw_rectangle(display, x+8, y+8, 16, h - 16, UI_COLOR_WHITE, 1);
				display->fill_area(display, UI_COLOR_BLACK, display->set_area(display, x + 9, y + 9, 14, h - (16+2)));
				xlen = h - (16+2);
				x1 = (((ui_gauge *)obj)->percent * xlen) / 100;
				//display->fill_area(display, UI_COLOR_GREY, display->set_area(display, x+3, y+3, xlen, h-6));
				display->fill_area(display, UI_COLOR_SLATE_GREY, display->set_area(display, x + 9, y + 9 + (h - (16+2)) - x1, 14, x1));
				break;
		}
		//display->print_string(display, UI_FONT_DEFAULT, x + xx, y, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
	}
}

void ui_slider_click(ui_object * obj, void * display) {
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 cx, cy, center;
	ui_gauge * gauge = (ui_gauge *)obj;
	uint16 gap ;
	if_touch_get(display, &cx, &cy);
	//implement touch slide (TODO)
	switch(gauge->orientation) {
			case UI_GAUGE_HORIZONTAL:
				gap = cx - x;
				gauge->percent = ((gap * 100) / w);
				if(gauge->percent < 0) gauge->percent = 0;
				else if(gauge->percent > 100)gauge->percent = 100;
				ui_reinitialize_object(obj);
				if(((ui_slider *)obj)->changed != NULL) ((ui_slider *)obj)->changed(obj, display);
				break;
			case UI_GAUGE_VERTICAL:
				gap = cy - (y + h);
				gauge->percent = 100 - ((gap * 100) / h);
				if(gauge->percent < 0) gauge->percent = 0;
				else if(gauge->percent > 100)gauge->percent = 100;
				ui_reinitialize_object(obj);
				if(((ui_slider *)obj)->changed != NULL) ((ui_slider *)obj)->changed(obj, display);
				break;
	}
	
}

ui_object * ui_slider_create(char * text, uint8 id, uint8 orientation, uint16 value, void (* changed)(ui_object *, void *)) {
	ui_gauge * obj;
	switch(orientation) {
		case UI_GAUGE_HORIZONTAL:
			obj = (ui_gauge *)ui_create_text(sizeof(ui_slider) + strlen((const char *)text), UI_TYPE_LABEL | UI_ALIGN_VERTICAL, (char *)text) ;
			((ui_object *)obj)->rect.h = 24;
			break;
		default:
		case UI_GAUGE_VERTICAL:
			obj = (ui_gauge *)ui_create_text(sizeof(ui_slider) + strlen((const char *)text), UI_TYPE_LABEL | UI_ALIGN_HORIZONTAL, (char *)text) ;
			((ui_object *)obj)->rect.w = 24;
			break;
	}
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(23,24,36);
	((ui_object *)obj)->render = ui_slider_render;
	((ui_object *)obj)->handler = ui_slider_click;
	((ui_gauge *)obj)->orientation = orientation;
	((ui_slider *)obj)->id = id;
	((ui_slider *)obj)->changed = changed;
	if(value < 100) ((ui_gauge *)obj)->percent = value; 
	else ((ui_gauge *)obj)->percent = 100; 
	return (ui_object *)obj;
}
