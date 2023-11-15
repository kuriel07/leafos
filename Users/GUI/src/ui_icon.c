#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_icon.h"

static void ui_icon_render(ui_object * obj, gui_handle_p display) {
	uint16 nc, wtxt, xx, yy;
	uint8 ww, hh, num_pal;
	uint8 i, j, c;
	uint32 palletes[8];
	uint16 offset;
	uint32 color;
	ui_object temp;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 font_size = UI_FONT_DEFAULT;
	if(((ui_rect *)obj)->w <= 40) font_size = UI_FONT_SMALL;
	if(obj->state & UI_STATE_INIT) {
		//display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		display->set_area(display, x, y, w, h);
		//print icon background first (white color)
		xx = ((w - 52) >> 1);
		display->fill_area(display, UI_COLOR_WHITE, display->set_area(display, x + xx, y + 2, 52, 52));
		nc = strlen((const char *)((ui_text *)obj)->text);
		wtxt = nc * 8;
		xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
		display->set_area(display, x, y, w, h);
		//display->print_string(display, UI_FONT_DEFAULT, x + xx, (y + h) - UI_FONT_DEFAULT, ((ui_object *)obj)->text, ((ui_object *)obj)->forecolor);
		display->print_string(display, UI_FONT_DEFAULT, x + xx, (y + h) - font_size, (uint8 *)((ui_text *)obj)->text, UI_COLOR_WHITE);
		if(((ui_icon *)obj)->icolen != 0) {
            ww = ((ui_icon *)obj)->bitmap[6];       //icon width
            hh = ((ui_icon *)obj)->bitmap[7];      //icon height
			memcpy(&temp, obj, sizeof(ui_object));
			((ui_rect *)&temp)->x += ((w - ww) >> 1);		//align top middle
			((ui_rect *)&temp)->y += 4;	
			temp.backcolor = UI_COLOR_WHITE;
			ui_resource_render(display, (ui_rect *)&temp, &((ui_object *)obj)->image, UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		}
	}
}

ui_object * ui_icon_create(uchar * text, uint8 aidlen, uint8 * aid, uint16 bmsize, uint8 * bitmap, void (* click)(ui_object *, void * params)) {
	ui_icon * obj = (ui_icon *)ui_create_text(sizeof(ui_icon) + bmsize, UI_TYPE_GRIDITEM | UI_ALIGN_GRID, (char *)text) ;
	if(aidlen <= 0x10) {
		obj->aidlen = aidlen;
		memcpy(obj->aid, aid, aidlen);
	}
	((ui_object *)obj)->rect.w = 64;
	((ui_object *)obj)->rect.h = 72;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_SLATE_GREY;
	((ui_object *)obj)->render = ui_icon_render;
	if(click != NULL) ((ui_object *)obj)->handler = click;
	//copy bitmap from memory
	memcpy(((ui_object *)obj)->image.buffer, bitmap, bmsize);
	((ui_object *)obj)->image.size = bmsize;
	return (ui_object *)obj;
}