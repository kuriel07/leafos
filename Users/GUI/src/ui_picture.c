#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_core.h"
#include "..\inc\ui_picture.h"

static void ui_picture_render(ui_object * obj, gui_handle_p display) {
	uint16 nc, wtxt, xx, yy;
	uint16 ww, hh, num_pal;
	uint8 i, j, c;
	uint16 offset;
	uint32 color;
	ui_object temp;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	if(obj->state & UI_STATE_INIT) {
		//display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		//display->print_string(display, UI_FONT_DEFAULT, x + xx, (y + h) - UI_FONT_DEFAULT, ((ui_object *)obj)->text, ((ui_object *)obj)->forecolor);
		if(((ui_object *)obj)->image.size != 0 && ((ui_object *)obj)->image.buffer != NULL) {
			display->set_area(display, x, y, w, h);
			if(ui_image_info(&((ui_object *)obj)->image, display->fb_pxfmt, &ww, &hh) != 0) return;
			((ui_rect *)obj)->h = hh + 4;
			
			//calculate and print image
            memcpy(&temp, obj, sizeof(ui_object));
			xx = ((w - ww) >> 1);
			display->set_area(display, x + xx, y + 2, ww, hh);
			((ui_rect *)&temp)->x = x + xx; 
			((ui_rect *)&temp)->y = y + 2;
			temp.backcolor = (obj->parent)->backcolor;		//use parent back color
			ui_resource_render(display, (ui_rect *)&temp, &((ui_object *)obj)->image, UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
			
			//print image description (if any)
			nc = strlen((const char *)((ui_text *)obj)->text);
			if(nc != 0) {
				wtxt = nc * 8;
				xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
				display->set_area(display, x, y, w, h);
				display->print_string(display, UI_FONT_DEFAULT, x + xx, (y + h) - UI_FONT_DEFAULT, (uint8 *)((ui_text *)obj)->text, UI_COLOR_WHITE);
				((ui_rect *)obj)->h += UI_FONT_DEFAULT;
			}
		}
	}
}

ui_object * ui_picture_create(uchar * text, uint16 pxfmt, uint16 bmsize, uint8 * bitmap, void (* click)(ui_object *, void * params)) {
	ui_picture * obj = (ui_picture *)ui_create_text(sizeof(ui_picture) + bmsize, UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, (char *)text) ;
	((ui_object *)obj)->rect.h = 72;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_SLATE_GREY;
	((ui_object *)obj)->render = ui_picture_render;
	if(click != NULL) ((ui_object *)obj)->handler = click;
	//copy bitmap from memory
	((ui_object *)obj)->image.buffer = os_alloc(bmsize);
	memcpy(((ui_object *)obj)->image.buffer, bitmap, bmsize);
	((ui_object *)obj)->image.size = bmsize;
	((ui_object *)obj)->img_release = os_free;
	return (ui_object *)obj;
}