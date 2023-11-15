#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_label.h"
#include "..\inc\ui_core.h"

static void ui_label_render(ui_object * obj, gui_handle_p display) {
	uint16 xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 i;
	uint16 j;
	uint16 index = 0;
	uint8 c;
	uint16 auto_h = 0;
	uint8 linebuf[31];
	if(obj->state & UI_STATE_INIT) {
		display->set_area(display, x, y, w, h);
		//display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		xx = 2;		//x = width - text_width / 2;
		//yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
		yy = 2;
		for(i=0;i<10;i++) {
			memset(linebuf, 0, sizeof(linebuf));
			for(j=0;j<(sizeof(linebuf) - 1);j++) {
				c = ((ui_label *)obj)->content[index++];
				if(c == '\n') break;
				if(c == 0) { i = 10; break; }
				linebuf[j] = c;
			}
			//j = i * (sizeof(linebuf)-1);
			//memcpy(linebuf, (uint8 *)(((ui_label *)obj)->content) + j, sizeof(linebuf) - 1);
			//linebuf[sizeof(linebuf) -1] = 0;
			display->print_string(display, ((ui_label *)obj)->font_size, x + xx, y + yy, (uint8 *)linebuf, ((ui_object *)obj)->forecolor);
			//if(strlen((const char *)linebuf) < (sizeof(linebuf) -1)) break;
			yy += ((ui_label *)obj)->font_size;
			auto_h += ((ui_label *)obj)->font_size;
		}
		((ui_rect *)obj)->h = (auto_h + 4);
		obj->rect.h = (auto_h + 4);
	}
}

ui_object * ui_label_create(DWORD color, uint8 numline, uint16 fontsize, uchar * text) {
	ui_label * obj = (ui_label *)ui_create_object(sizeof(ui_label) + strlen((const char *)text), UI_TYPE_LABEL | UI_ALIGN_VERTICAL) ;
	((ui_object *)obj)->rect.h = (numline * fontsize) + 4;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(23,24,36);
	((ui_object *)obj)->render = ui_label_render;
	((ui_object *)obj)->handler = NULL;
	obj->font_size = fontsize;
	strcpy((char *)((ui_label *)obj)->content, (const char *)text);
	return (ui_object *)obj;
}

ui_object * ui_dynamic_label_create(DWORD color, uint8 numline, uint16 fontsize, uchar * text) {
	ui_label * obj = (ui_label *)ui_create_text(sizeof(ui_label) + UI_MAX_TEXT, UI_TYPE_LABEL | UI_ALIGN_VERTICAL, (char *)text) ;
	((ui_object *)obj)->rect.h = (numline * fontsize) + 4;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(23,24,36);
	((ui_object *)obj)->render = ui_label_render;
	((ui_object *)obj)->handler = NULL;
	obj->font_size = fontsize;
	return (ui_object *)obj;
}