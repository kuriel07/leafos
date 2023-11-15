#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_panel.h"

void ui_panel_render(ui_object * obj, gui_handle_p display) {
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	if(obj->state & UI_STATE_INIT) {
		if(((ui_panel *)obj)->radius != 0) {
			display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x + 1, y + 1, w - 2, h - 2));
		} else {
			display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		}
	}
	//display->print_string(display, UI_FONT_DEFAULT, x, (y + h) - UI_FONT_DEFAULT, ((ui_object *)obj)->text, ((ui_object *)obj)->forecolor);
}

static void ui_toolbar_render(ui_object * obj, gui_handle_p display) {
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	if(obj->state & UI_STATE_INIT) {
		display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		//display separator border
		//display->fill_area(display, UI_COLOR_RGB(57,59,67), display->set_area(display, x + w-4, y, 1, h));
		//display->fill_area(display, UI_COLOR_RGB(49,51,59), display->set_area(display, x + w-3, y, 1, h));
		//display->fill_area(display, UI_COLOR_RGB(79,81,89), display->set_area(display, x + w-2, y, 1, h));
		//display->fill_area(display, UI_COLOR_RGB(60,62,70), display->set_area(display, x + w-1, y, 1, h));
#if 0
		display->fill_area(display, 0xFF949294, display->set_area(display, x, y + 0, w, 1));
		display->fill_area(display, 0xFF64727C, display->set_area(display, x, y + 1, w, 1));
		display->fill_area(display, 0xFF64717C, display->set_area(display, x, y + 2, w, 1));
		display->fill_area(display, 0xFF616C79, display->set_area(display, x, y + 3, w, 1));
		display->fill_area(display, 0xFF5C6A74, display->set_area(display, x, y + 4, w, 1));
		display->fill_area(display, 0xFF5C6774, display->set_area(display, x, y + 5, w, 1));
		display->fill_area(display, 0xFF556274, display->set_area(display, x, y + 6, w, 1));
		display->fill_area(display, 0xFF545F6D, display->set_area(display, x, y + 7, w, 1));
		display->fill_area(display, 0xFF535D6C, display->set_area(display, x, y + 8, w, 1));
		display->fill_area(display, 0xFF4C596A, display->set_area(display, x, y + 9, w, 1));
		display->fill_area(display, 0xFF444F5E, display->set_area(display, x, y + 10, w, 1));
		display->fill_area(display, 0xFF36424D, display->set_area(display, x, y + 11, w, 1));
		display->fill_area(display, 0xFF32414D, display->set_area(display, x, y + 12, w, 1));
		display->fill_area(display, 0xFF313E4E, display->set_area(display, x, y + 13, w, 1));
		display->fill_area(display, 0xFF2D3A4C, display->set_area(display, x, y + 14, w, 1));
		display->fill_area(display, 0xFF2C364B, display->set_area(display, x, y + 15, w, 1));
		display->fill_area(display, 0xFF2C354A, display->set_area(display, x, y + 16, w, 1));
		display->fill_area(display, 0xFF2B3340, display->set_area(display, x, y + 17, w, 1));
		display->fill_area(display, 0xFF283448, display->set_area(display, x, y + 18, w, 1));
		display->fill_area(display, 0xff273240, display->set_area(display, x, y + 19, w, 1));
		display->fill_area(display, 0xff272D3C, display->set_area(display, x, y + 20, w, 1));
		display->fill_area(display, 0xff242A3C, display->set_area(display, x, y + 21, w, 2));
		display->fill_area(display, 0xff151F2D, display->set_area(display, x, y + 23, w, 1));
#endif
	}
}

void ui_tab_render(ui_object * obj, gui_handle_p display) {
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 yy, xx, hh, ww;
	uint32 fcolor;
	uint8 c;
	uint16 halfh = (((ui_rect *)obj)->h / 2);
	
	//generate background first
	if(obj->state & UI_STATE_KEYDOWN) {
		display->fill_area(display, UI_COLOR_QUEEN_BLUE, display->set_area(display, x, y, w, h));
		obj->backcolor = UI_COLOR_QUEEN_BLUE;
	} else {
		display->fill_area(display, UI_COLOR_RGB(61,63,71), display->set_area(display, x, y, w, h));
		obj->backcolor = UI_COLOR_RGB(61,63,71);
	}
	if(display->orientation & 0x01) {
		//display->fill_area(display, UI_COLOR_RGB(57,59,67), display->set_area(display, x, y, 1, h));
		//display->fill_area(display, UI_COLOR_RGB(49,51,59), display->set_area(display, x + 1, y, 1, h));
		display->fill_area(display, UI_COLOR_RGB(79,81,89), display->set_area(display, x + w - 2, y, 1, h));
		display->fill_area(display, UI_COLOR_RGB(60,62,70), display->set_area(display, x + w - 1, y, 1, h));
	} else {
		//display separator border (bottom)
		//display->fill_area(display, UI_COLOR_RGB(57,59,67), display->set_area(display, x, y, w, 1));
		//display->fill_area(display, UI_COLOR_RGB(49,51,59), display->set_area(display, x, y + 1, w, 1));
		display->fill_area(display, UI_COLOR_RGB(79,81,89), display->set_area(display, x, y + h-2, w, 1));
		display->fill_area(display, UI_COLOR_RGB(60,62,70), display->set_area(display, x, y + h-1, w, 1));
	}
}

ui_object * ui_window_create(gui_handle_p display, uint8 * text) {
	ui_object * panel = ui_create_panel(sizeof(ui_panel), 60, 24, display->width - 60, display->height -24, (uint8 *)text, 0, 0);
	if(display->orientation & 0x01) {
		ui_set_position(panel, 0, 24);
		ui_set_size(panel, ((gui_handle_p)display)->width, (((gui_handle_p)display)->height - 24) - 60);
			
	} else {
		ui_set_position(panel, 60, 24);
		ui_set_size(panel, ((gui_handle_p)display)->width - 60, ((gui_handle_p)display)->height - 24);
	}
	return panel;
}

ui_object * ui_toolbar_create(uint16 x, uint16 y, uint16 w, uint16 h) {
	ui_panel * obj = (ui_panel *)ui_create_object(sizeof(ui_panel), UI_TYPE_LABEL | UI_ALIGN_FLOAT) ;
	((ui_object *)obj)->rect.x = x;
	((ui_object *)obj)->rect.y = y;
	((ui_object *)obj)->rect.w = w;
	((ui_object *)obj)->rect.h = h;
	((ui_object *)obj)->forecolor = UI_COLOR_BLACK;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(58,60,66);	//UI_COLOR_RGB(61,63,71);
	((ui_object *)obj)->render = ui_toolbar_render;
	((ui_object *)obj)->handler = NULL;
	obj->border = 0;
	obj->radius = 0;
	return (ui_object *)obj;
}

ui_object * ui_panel_create(uint32 align, uint16 w, uint16 h) {
	ui_panel * obj = (ui_panel *)ui_create_object(sizeof(ui_panel), UI_TYPE_PANEL);
	((ui_object *)obj)->rect.x = 0;
	((ui_object *)obj)->rect.y = 0;
	((ui_object *)obj)->rect.w = w;
	((ui_object *)obj)->rect.h = h;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = 0;
	((ui_object *)obj)->render = ui_panel_render;
	((ui_object *)obj)->handler = NULL;
	obj->thumbnail = NULL;
	obj->border = 0;
	obj->radius = 0;
	ui_set_size((ui_object *)obj, w, h);
	ui_set_align((ui_object *)obj, align);
	return (ui_object *)obj;
}

ui_object * ui_create_panel(size_t objsize, uint16 x, uint16 y, uint16 w, uint16 h, uint8 * text, uint8 border, uint8 radius) {
	ui_panel * obj = (ui_panel *)ui_create_text(objsize, UI_TYPE_LABEL | UI_TYPE_PANEL | UI_ALIGN_FLOAT, (char *)text) ;
	((ui_object *)obj)->rect.x = x;
	((ui_object *)obj)->rect.y = y;
	((ui_object *)obj)->rect.w = w;
	((ui_object *)obj)->rect.h = h;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = 0;
	((ui_object *)obj)->render = ui_panel_render;
	((ui_object *)obj)->handler = NULL;
	obj->thumbnail = NULL;
	obj->border = border;
	obj->radius = radius;
	return (ui_object *)obj;
}
