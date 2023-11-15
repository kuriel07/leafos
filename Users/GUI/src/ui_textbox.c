#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_textbox.h"
#include "..\inc\ui_keyboard.h"


static void ui_textbox_render(ui_object * obj, gui_handle_p display) {
	uint8 buffer[40];
	uint8 i; size_t j;
	uint16 cx, cy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 charline = (w / 8) - 1;			//number of character perline
	uint16 chartotal = strlen((const char *)((ui_textbox *)obj)->content);		//calculate total characters
	if(obj->state & UI_STATE_INIT) {
		display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x+3, y+4, w-6, h-8));
		display->draw_rectangle(display, x+2, y+3, w-4, h-6, UI_COLOR_BLACK, 1);
		j = charline;
		display->set_area(display, x+3, y+4, w-6, h-8);
		for(i = 0 ; i < chartotal; i += charline, y += UI_FONT_DEFAULT) {
			//print perline
			if((i + charline) > chartotal) j = chartotal - i;
			if((((ui_textbox *)obj)->mode & 0x80) == 0 &&  (((ui_textbox *)obj)->mode & 0x04) != 0 ) {
				//masked textbox
				memset(buffer, '*', j);
				buffer[j] = 0;
				display->print_string(display, UI_FONT_DEFAULT, x + 4, y + 4, buffer, ((ui_object *)obj)->forecolor);
			} else {
				//non-masked textbox/special character
				memcpy(buffer, (const void *)(((ui_textbox *)obj)->content + i), j);
				buffer[j] = 0;
				display->print_string(display, UI_FONT_DEFAULT, x + 4, y + 4, buffer, ((ui_object *)obj)->forecolor);
			}
		}
	}
}

void ui_textbox_handler(ui_object * obj, void * display) {
	if(obj->state & UI_STATE_SELECTED) {
		if(((ui_textbox *)obj)->mode & 0x80) {
			//special char textbox
			ui_keyboard_show(display, (ui_textbox *)obj, ((ui_textbox *)obj)->mode & 0x0F);
		} else {
			//normal textbox (configuration followint ETSI223)
			switch(((ui_textbox *)obj)->mode & 0x03) {
				case UI_TEXTBOX_ALPHANUM: 
					ui_keyboard_show(display, (ui_textbox *)obj, UI_KEYBOARD_LOWERCASE);
					break;
				case UI_TEXTBOX_NUMERIC:
					ui_keyboard_show(display, (ui_textbox *)obj, UI_KEYBOARD_NUMERIC);
					break;
			}
		}
	}
}

void ui_set_content(ui_object * obj, char * content) {
	if((((ui_object *)obj)->type & (UI_TYPE_SELECTABLE | UI_TYPE_TEXT)) == (UI_TYPE_SELECTABLE | UI_TYPE_TEXT)) {
		strncpy((char *)((ui_textbox *)obj)->content, (const char *)content, UI_MAX_TEXT);
	}
}

ui_object * ui_textbox_create(uint8 mode, uchar * default_text, uint8 maxlen, uint16 numline) {
	ui_textbox * obj = (ui_textbox *)ui_create_object(sizeof(ui_textbox), UI_TYPE_SELECTABLE | UI_ALIGN_VERTICAL | UI_TYPE_TEXT) ;
	((ui_textbox *)obj)->maxlen = maxlen;
	((ui_textbox *)obj)->numline = 1;
	((ui_textbox *)obj)->duration = 0;
	((ui_textbox *)obj)->mode = mode;
	((ui_textbox *)obj)->blinked = UI_TEXTBOX_BLINK_ENABLED;
	((ui_object *)obj)->rect.h = 40;		//(UI_FONT_DEFAULT * numline) + 32;
	((ui_object *)obj)->render = ui_textbox_render;
	((ui_object *)obj)->forecolor = UI_COLOR_BLACK;
	((ui_object *)obj)->backcolor = UI_COLOR_WHITE;
	((ui_object *)obj)->handler = ui_textbox_handler;
	strncpy((char *)((ui_textbox *)obj)->content, (const char *)default_text, UI_MAX_TEXT);
	((ui_textbox *)obj)->curidx = strlen((const char *)default_text);
	return (ui_object *)obj;
}