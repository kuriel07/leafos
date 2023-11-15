#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_combo.h"
#include "..\inc\ui_panel.h"
#include "..\inc\ui_item.h"
#include "..\inc\ui_resources.h"
#include <stdarg.h>
#define MAX_ARG_SIZE		16

static void ui_combo_item_render(ui_object * obj, gui_handle_p display) {
	uint16 nc, wtxt, xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	if(obj->state & UI_STATE_INIT) {
		//display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		display->set_area(display, x, y, w, h);
		//display->draw_line(display, x, y, x+w, y, UI_COLOR_BLACK);
		//display->draw_line(display, x, y+h-1, x+w, y+h-1, UI_COLOR_WHITE);
		xx = 4;		//x = width - text_width / 2;
		yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
		display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
	}
}

static void ui_combo_render(ui_object * obj, gui_handle_p display) {
	uint16 nc, wtxt, xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	ui_object frame;
	if(obj->state & UI_STATE_INIT) {
		memcpy(&frame, obj, sizeof(ui_object));
		//display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		nc = strlen((const char *)((ui_text *)obj)->text);
		wtxt = nc * 8;
		xx = 4;		//x = width - text_width / 2;
		yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
		display->set_area(display, x, y, w, h);
		//display->draw_rectangle(display, x + 2, y + 2, w - 4, h - 4, UI_COLOR_BLACK, 1);
		display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x, y, w, h));
		
		display->fill_area(display, UI_COLOR_RGB(57,59,67), display->set_area(display, x, y + h-4, w, 1));
		display->fill_area(display, UI_COLOR_RGB(49,51,59), display->set_area(display, x, y + h-3, w, 1));
		display->fill_area(display, UI_COLOR_RGB(79,81,89), display->set_area(display, x, y + h-2, w, 1));
		display->fill_area(display, UI_COLOR_RGB(60,62,70), display->set_area(display, x, y + h-1, w, 1));
#if 0
		display->fill_area(display, 0xFFfefefe, display->set_area(display, x + 3, y + 3, w - 6, 4));
		display->fill_area(display, 0xFFfcfcfc, display->set_area(display, x +3, y + 7, w - 6, 3));
		display->fill_area(display, 0xFFf5f5f5, display->set_area(display, x + 3, y + 10, w - 6, 2));
		display->fill_area(display, 0xFFf0f0f0, display->set_area(display, x + 3, y + 12, w - 6, 1));
		display->fill_area(display, 0xFFeeeeee, display->set_area(display, x + 3, y + 13, w - 6, 1));
		display->fill_area(display, 0xFFe5e5e5, display->set_area(display, x + 3, y + 14, w - 6, 1));
		display->fill_area(display, 0xFFdedede, display->set_area(display, x + 3, y + 15, w - 6, 1));
		display->fill_area(display, 0xFFd9d9d9, display->set_area(display, x + 3, y + 16, w - 6, 1));
		display->fill_area(display, 0xffd4d4d4, display->set_area(display, x + 3, y + 17, w - 6, 1));
		display->fill_area(display, 0xffcfcfcf, display->set_area(display, x + 3, y + 18, w - 6, 1));
		display->fill_area(display, 0xffcacaca, display->set_area(display, x + 3, y + 19, w - 6, 1));
		display->fill_area(display, 0xffc6c6c6, display->set_area(display, x + 3, y + 20, w - 6, 1));
#endif
		display->set_area(display, x, y, w, h);
		//display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)">>", ((ui_object *)obj)->forecolor);
		((ui_rect *)&frame)->x = x + xx;
		((ui_rect *)&frame)->y = y + yy;
		((ui_rect *)&frame)->w = 16;
		((ui_rect *)&frame)->h = 16;
		ui_image_render(display, (ui_rect *)&frame, (uint8 *)image_png_pin16, sizeof(image_png_pin16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		xx += 20;
		display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
	}
}

static void ui_option_click(ui_object * obj, void * params) {
	ui_combo * cbox = (ui_combo *)obj->target;
	uint8 i;
	uint16 len = 0;
	((ui_combo *)obj->target)->index =  ((ui_item *)obj)->id;
	((ui_text *)cbox)->text[0] = 0;
	for(i=0;i<cbox->total;i++) {
		if(i == ((ui_item *)obj)->id) strncpy((char *)((ui_text *)cbox)->text, (const char *)cbox->content + len, UI_MAX_TEXT);
		len += strlen((const char *)cbox->content + len) + 1;
	}
	ui_reinitialize_object(obj->target);
	ui_pop_screen((gui_handle_p)params);
	//trigger handler
	if(cbox->selected != NULL) cbox->selected((ui_object *)cbox, params);
}

static ui_object * ui_combo_item_create(uchar * text, uint8 id, void (* click)(ui_object *, void *)) {
	ui_item * obj = (ui_item *)ui_create_text(sizeof(ui_item), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, (char *)text) ;
	obj->id = id;
	((ui_object *)obj)->rect.h = 48;
	((ui_object *)obj)->render = ui_combo_item_render;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(79,81,89);
	((ui_object *)obj)->handler = click;
	return (ui_object *)obj;
}

static void ui_option_show(gui_handle_p display, ui_combo * obj) {
	uint8 i;
	ui_object * panel;
	uint16 len = 0;
	ui_object * item;
	ui_object * body = ((gui_handle_p)display)->body;
	panel = ui_create_panel(sizeof(ui_panel), ((ui_rect *)obj)->x, ((ui_rect *)obj)->y + ((ui_rect *)obj)->h, ((ui_rect *)obj)->w, 48 * obj->total, (uint8 *)"", 1, 1);
	panel->backcolor = UI_COLOR_RGB(79,81,89);
	len = 0;
	for(i=0;i<obj->total;i++) {
		ui_add_object(panel, (item = ui_combo_item_create(obj->content + len, i, ui_option_click)));
		item->target = (ui_object *)obj;
		len += strlen((const char *)obj->content + len) + 1;
	}
	ui_push_screen(display, panel);
}

static void ui_combo_click(ui_object * obj, void * params) {
	ui_option_show(params, (ui_combo *)obj);
}

ui_object * ui_combo_create(uint8 index, uint8 total, uint8 * text, void (* selected)(ui_object *, void *), ...) {
	ui_combo * obj;
	va_list arg_ptr;
	//uint8 * ptr;
    uint8 * ptr[MAX_ARG_SIZE];
	uint16 len = 0;
	uint8 i;
	va_start(arg_ptr, selected);
	for(i=0;i<total;i++) {
		ptr[i] = (uint8 *)va_arg(arg_ptr, uint8 *);
		if(ptr[i] == NULL) break;
		len += strlen((const char *)ptr[i]) + 1;
	}
	va_end(arg_ptr);
	obj = (ui_combo *)ui_create_text(sizeof(ui_combo) + len, UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, "") ;
	len = 0;
	for(i=0;i<total;i++) {
		strcpy((char *)(obj->content + len), (const char *)ptr[i]);
		if(i == index) strncpy((char *)((ui_text *)obj)->text, (const char *)ptr[i], UI_MAX_TEXT);
		len += strlen((const char *)ptr[i]) + 1;
	}
	obj->total = total;
	((ui_object *)obj)->render = ui_combo_render;
	((ui_object *)obj)->rect.h = 48;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(61,63,71);
	obj->index = index;
	((ui_object *)obj)->handler = ui_combo_click;
	obj->selected = selected;
	return (ui_object *)obj;
}