#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_toolbox.h"
#include "..\inc\ui_resources.h"
#include "..\inc\ui_setting.h"
#include "..\inc\ui_item.h"
#include "..\..\interfaces\inc\if_apis.h"

static void ui_tristate_item_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_tristate_item_render);
	uint16 nc, wtxt, xx, yy;
	uint8 ww, hh, num_pal;
	uint8 i, j, c;
	uint32 palletes[8];
	uint16 offset;
	uint32 color;
	uint8 index;
	uint8 * objmp;
	ui_state_item * subitem;
	ui_object frame;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	subitem = &((ui_tristate_item *)obj)->items[((ui_toggle *)obj)->state & 0x03];
	if(subitem != NULL) {
		if(obj->state != 0) {
			((ui_object *)obj)->image.buffer = subitem->bitmap;
			((ui_object *)obj)->image.size = subitem->bmpsize;
			ui_item_render(obj, display);
			if(w > 64) {
				nc = strlen((const char *)subitem->text);
				wtxt = nc * 8;
				xx = ((ui_rect *)obj)->h + 4;
				yy = (h - UI_FONT_DEFAULT) >> 1;
				display->set_area(display, x, y, w, h);
				display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, subitem->text, UI_COLOR_WHITE);
			} else {
				xx = 4;
				yy = h - (UI_FONT_SMALL + 2);
				display->print_string(display, UI_FONT_SMALL, x + xx, y + yy, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
			}
		}
	}
	OS_DEBUG_EXIT();
}

static void ui_config_item_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_config_item_render);
	uint16 nc, wtxt, xx, yy;
	uint8 ww, hh, num_pal;
	uint8 i, j, c;
	uint32 palletes[8];
	uint16 offset;
	uint32 color;
	uint8 index;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	ui_icon_bitmap * objmp = &((ui_icon_tool *)obj)->bitmap[((ui_toggle *)obj)->state & 0x01];
	
	if(obj->state != 0) {
		display->set_area(display, x, y, w, h);
		if(objmp != NULL) {
			ui_image_render(display, (ui_rect *)obj, objmp->bitmap, objmp->bmpsize, UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		}
		nc = strlen((const char *)((ui_text *)obj)->text);
		wtxt = nc * 8;
		xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
		display->set_area(display, x, y, w, h);
		display->print_string(display, UI_FONT_DEFAULT, x + xx, (y + h) - UI_FONT_DEFAULT, (uint8 *)((ui_text *)obj)->text, UI_COLOR_WHITE);
	}
	OS_DEBUG_EXIT();
}

ui_object * ui_autobtn_create(uint8 * name, void * target, void (* click)(ui_object *, void *)) {
	ui_tristate_item * obj = (ui_tristate_item *)ui_create_item(sizeof(ui_tristate_item), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, "") ;
	((ui_object *)obj)->rect.h = 48;
	strcpy((char *)((ui_object *)obj)->name, (const char *)name);
	((ui_object *)obj)->render = ui_tristate_item_render;
	((ui_object *)obj)->forecolor = UI_COLOR_GREY;
	((ui_object *)obj)->backcolor = UI_COLOR_QUEEN_BLUE;
	((ui_object *)obj)->target = target;
	((ui_object *)obj)->handler = click;
	((ui_tristate_item *)obj)->state = 0;		//toggle off, enabled
	obj->items[0].bitmap = (uint8 *)image_png_play_off24;		//(uint8 *)img_robot_normal;
	obj->items[0].bmpsize = sizeof(image_png_play_off24);			//sizeof(img_robot_normal);
	obj->items[0].text = (uint8 *)"Automaton : Off";
	obj->items[1].bitmap = (uint8 *)image_png_play_on24;		//(uint8 *)img_robot_work;
	obj->items[1].bmpsize = sizeof(image_png_play_on24);			//sizeof(img_robot_work);
	obj->items[1].text = (uint8 *)"Automaton : Run";
	obj->items[2].bitmap = (uint8 *)image_png_rec_on24;		//(uint8 *)img_robot_work;
	obj->items[2].bmpsize = sizeof(image_png_rec_on24);			//sizeof(img_robot_work);
	obj->items[2].text = (uint8 *)"Automaton : Record";
	return (ui_object *)obj;
}
