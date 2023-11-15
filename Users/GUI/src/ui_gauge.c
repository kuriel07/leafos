#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_gauge.h"
#include "..\inc\ui_core.h"

static void ui_gauge_render(ui_object * obj, gui_handle_p display) {
	uint16 xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 i;
	uint16 j;
	uint16 xlen, x1, wtxt, nc;
	uint8 linebuf[27];
	if(obj->state & UI_STATE_INIT) {
		//draw border
		display->draw_rectangle(display, x+1, y+1, w-2, h-2, UI_COLOR_WHITE, 1);
		//draw content
		display->set_area(display, x + 3, y + 3, w - 6, h - 6);
		xlen = w - 6;
		x1 = (((ui_gauge *)obj)->percent * xlen) / 100;
		//display->fill_area(display, UI_COLOR_GREY, display->set_area(display, x+3, y+3, xlen, h-6));
		display->fill_area(display, UI_COLOR_SLATE_GREY, display->set_area(display, x+3, y+3, x1, h-6));
		nc = strlen((const char *)((ui_text *)obj)->text);		//calculate total characters
		wtxt = nc * 8;
		xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
		display->set_area(display, x, y, w, h);
		display->print_string(display, UI_FONT_DEFAULT, x + xx, y, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
	}
}

ui_object * ui_gauge_create(DWORD color, uint8 percent, uchar * text) {
	ui_gauge * obj = (ui_gauge *)ui_create_text(sizeof(ui_gauge) + strlen((const char *)text), UI_TYPE_LABEL | UI_ALIGN_VERTICAL, (char *)text) ;
	((ui_object *)obj)->rect.h = 16;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(23,24,36);
	((ui_object *)obj)->render = ui_gauge_render;
	((ui_object *)obj)->handler = NULL;
	((ui_gauge *)obj)->percent = percent;
	return (ui_object *)obj;
}


static void ui_infostat_render(ui_object * obj, gui_handle_p display) {
	uint16 xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 i;
	uint16 j;
	uint16 xlen, x1, wtxt, nc;
	uint8 linebuf[27];
	if(obj->state & UI_STATE_INIT) {
		
	}
}

ui_object * ui_infostat_create(void * ctx) {
	ui_infostat * obj = (ui_infostat *)ui_create_text(sizeof(ui_infostat) + 128, UI_TYPE_LABEL | UI_ALIGN_FLOAT, "") ;
	((ui_object *)obj)->rect.h = 24;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(23,24,36);
	((ui_object *)obj)->render = ui_infostat_render;
	((ui_object *)obj)->handler = NULL;
	((ui_object *)obj)->target = ctx;
	obj->tick = 0;
	obj->cpu_util = 0;
	obj->fps = 0;
	obj->status = 0;
	return (ui_object *)obj;
}