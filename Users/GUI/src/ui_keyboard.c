#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "..\inc\ui_keyboard.h"
#include "..\inc\ui_button.h"
#include "..\inc\ui_panel.h"
#include "..\..\interfaces\inc\if_touch.h"
#include "..\..\interfaces\inc\if_apis.h"

//static ui_keybtn * g_ukeys[32];
ui_keybtn g_ba_keyset_1[] = {
	{ 0,0,1,1, 'Q' },
	{ 1,0,1,1, 'W' },
	{ 2,0,1,1, 'E' },
	{ 3,0,1,1, 'R' },
	{ 4,0,1,1, 'T' },
	{ 5,0,1,1, 'Y' },
	{ 6,0,1,1, 'U' },
	{ 7,0,1,1, 'I' },
	{ 8,0,1,1, 'O' },
	{ 9,0,1,1, 'P' },
	{ 0,1,1,1, 'A' },
	{ 1,1,1,1, 'S' },
	{ 2,1,1,1, 'D' },
	{ 3,1,1,1, 'F' },
	{ 4,1,1,1, 'G' },
	{ 5,1,1,1, 'H' },
	{ 6,1,1,1, 'J' },
	{ 7,1,1,1, 'K' },
	{ 8,1,1,1, 'L' },
	{ 0,2,1,1, 0x0F },			//shift out
	{ 1,2,1,1, 'Z' },
	{ 2,2,1,1, 'X' },
	{ 3,2,1,1, 'C' },
	{ 4,2,1,1, 'V' },
	{ 5,2,1,1, 'B' },
	{ 6,2,1,1, 'N' },
	{ 7,2,1,1, 'M' },
	{ 8,2,1,1, '@' },
	//{ 9,1,1,2, 0x08 },
	{ 0,3,2,1, 0x1A },
	{ 2,3,1,1, ',' },			//comma
	{ 3,3,4,1, ' ' },		//space
	{ 7,3,1,1, '.' },			//dot
	//{ 8,3,2,1, 0x0C },			//enter
	{ 8,3,2,1, 0x08 },			//enter
	{ 0,0,0,0, 0 }			//end mark
};

ui_keybtn g_ba_keyset_2[] = {
	{ 0,0,1,1, 'q' },
	{ 1,0,1,1, 'w' },
	{ 2,0,1,1, 'e' },
	{ 3,0,1,1, 'r' },
	{ 4,0,1,1, 't' },
	{ 5,0,1,1, 'y' },
	{ 6,0,1,1, 'u' },
	{ 7,0,1,1, 'i' },
	{ 8,0,1,1, 'o' },
	{ 9,0,1,1, 'p' },
	{ 0,1,1,1, 'a' },
	{ 1,1,1,1, 's' },
	{ 2,1,1,1, 'd' },
	{ 3,1,1,1, 'f' },
	{ 4,1,1,1, 'g' },
	{ 5,1,1,1, 'h' },
	{ 6,1,1,1, 'j' },
	{ 7,1,1,1, 'k' },
	{ 8,1,1,1, 'l' },
	{ 0,2,1,1, 0x0E },			//shift in
	{ 1,2,1,1, 'z' },
	{ 2,2,1,1, 'x' },
	{ 3,2,1,1, 'c' },
	{ 4,2,1,1, 'v' },
	{ 5,2,1,1, 'b' },
	{ 6,2,1,1, 'n' },
	{ 7,2,1,1, 'm' },
	{ 8,2,1,1, '@' },
	//{ 9,1,1,2, 0x08 },

	{ 0,3,2,1, 0x1A },
	{ 2,3,1,1, ',' },			//comma
	{ 3,3,4,1, ' ' },		//space
	{ 7,3,1,1, '.' },			//dot
	//{ 8,3,2,1, 0x0C },			//enter
	{ 8,3,2,1, 0x08 },			//enter
	{ 0,0,0,0, 0 }			//end mark
};

ui_keybtn g_ba_keyset_3[] = {
	{ 0,0,1,1, '1' },
	{ 1,0,1,1, '2' },
	{ 2,0,1,1, '3' },
	{ 3,0,1,1, '4' },
	{ 4,0,1,1, '5' },
	{ 5,0,1,1, '6' },
	{ 6,0,1,1, '7' },
	{ 7,0,1,1, '8' },
	{ 8,0,1,1, '9' },
	{ 9,0,1,1, '0' },
	{ 0,1,1,1, '-' },
	{ 1,1,1,1, '/' },
	{ 2,1,1,1, ':' },
	{ 3,1,1,1, ';' },
	{ 4,1,1,1, '(' },
	{ 5,1,1,1, ')' },
	{ 6,1,1,1, '$' },
	{ 7,1,1,1, '&' },
	{ 8,1,1,1, '%' },
	{ 0,2,1,1, 0x0E },
	{ 1,2,1,1, '?' },
	{ 2,2,1,1, '!' },
	{ 3,2,1,1, '\'' },
	{ 4,2,1,1, '\\' },
	{ 5,2,1,1, '+' },
	{ 6,2,1,1, '=' },
	{ 7,2,1,1, '_' },
	{ 8,2,1,1, '@' },
	//{ 9,1,1,2, 0x08 },
	{ 0,3,2,1, 0x0F },
	{ 2,3,1,1, ',' },			//comma
	{ 3,3,4,1, ' ' },		//space
	{ 7,3,1,1, '.' },			//dot
	//{ 8,3,2,1, 0x0C },			//enter
	{ 8,3,2,1, 0x08 },			//enter
	{ 0,0,0,0, 0 }			//end mark
};

ui_keybtn g_ba_keyset_4[] = {
	{ 2,0,2,1, '1' },
	{ 4,0,2,1, '2' },
	{ 6,0,2,1, '3' },
	{ 2,1,2,1, '4' },
	{ 4,1,2,1, '5' },
	{ 6,1,2,1, '6' },
	{ 2,2,2,1, '7' },
	{ 4,2,2,1, '8' },
	{ 6,2,2,1, '9' },
	{ 4,3,2,1, '0' },
	{ 6,3,2,1, 0x08 },
	{ 0,0,0,0, 0 }			//end mark
};

ui_keybtn g_ba_keyset_5[] = {
	{ 2,0,2,1, '1' },
	{ 4,0,2,1, '2' },
	{ 6,0,2,1, '3' },
	{ 2,1,2,1, '4' },
	{ 4,1,2,1, '5' },
	{ 6,1,2,1, '6' },
	{ 2,2,2,1, '7' },
	{ 4,2,2,1, '8' },
	{ 6,2,2,1, '9' },
	{ 2,3,2,1, '.' },
	{ 4,3,2,1, '0' },
	{ 6,3,2,1, 0x08 },
	{ 0,0,0,0, 0 }			//end mark
};
ui_keybtn * g_ba_keyset_keys[] = { g_ba_keyset_1, g_ba_keyset_2, g_ba_keyset_3, g_ba_keyset_4, g_ba_keyset_5 };

static void ui_keyboard_button_render(gui_handle_p display, uint16 x, uint16 y, uint16 w, uint16 h, uint16 state, uint8 c) {
	OS_DEBUG_ENTRY(ui_keyboard_button_render);
	uint16 nc, wtxt, xx, yy;
	uint8 cbuf[5];
	uint32 color = UI_COLOR_WHITE;
	ui_object rect;
	cbuf[0] = c;
	cbuf[1] = 0;
	((ui_rect *)&rect)->x = x;
	((ui_rect *)&rect)->y = y;
	((ui_rect *)&rect)->w = w;
	((ui_rect *)&rect)->h = h;
	((ui_object *)&rect)->backcolor = UI_COLOR_BLACK;
	if(state & UI_STATE_INIT) {
		//display->fill_area(display, UI_COLOR_RGB(58,60,66), display->set_area(display, x + 1, y + 1, w - 2, h - 2));
		ui_draw_button(display, &rect, UI_BUTTON_STATE_NONE);
		goto start_render;
	}
	if(state & UI_STATE_KEYDOWN) {
		//display->fill_area(display, UI_COLOR_QUEEN_BLUE, display->set_area(display, x + 1, y + 1, w - 2, h - 2));
		//display->set_area(display, x, y, w, h);
		ui_draw_button(display, &rect, UI_BUTTON_STATE_PRESSED);
		goto start_render;
	}
	if(state & UI_STATE_KEYUP) {
		//display->fill_area(display, UI_COLOR_RGB(58,60,66), display->set_area(display, x + 1, y + 1, w - 2, h - 2));
		ui_draw_button(display, &rect, UI_BUTTON_STATE_NONE);
		goto start_render;
	}
	start_render:
	switch(c) {
		case 0x0E:		//shift in
			xx -= 4;
			strcpy((char *)cbuf, "AZ");
			nc = strlen((const char *)cbuf);
			wtxt = nc * 8;
			xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
			yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
			goto print_normal;
		case 0x0F:		//shift out
			xx -= 4;
			strcpy((char *)cbuf, "az");
			nc = strlen((const char *)cbuf);
			wtxt = nc * 8;
			xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
			yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
			goto print_normal;
		case 0x1A:		//switch symbol
			xx -= 10;
			strcpy((char *)cbuf, "09#");
			nc = strlen((const char *)cbuf);
			wtxt = nc * 8;
			xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
			yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
			goto print_normal;
		case 0x0C:		//enter
			strcpy((char *)cbuf, "DONE");
			nc = strlen((const char *)cbuf);
			wtxt = nc * 6;
			xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
			yy = ((h - UI_FONT_SMALL) >> 1);		//y = height - text_height /2;
			goto print_small;
		case 0x08:		//back/delete
			strcpy((char *)cbuf, "BACK");
			nc = strlen((const char *)cbuf);
			wtxt = nc * 6;
			xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
			yy = ((h - UI_FONT_SMALL) >> 1);		//y = height - text_height /2;
			print_small:
			display->print_string(display, UI_FONT_SMALL, x + xx, y + yy, cbuf, color);
			break;
		default:
			nc = strlen((const char *)cbuf);
			wtxt = nc * 8;
			xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
			yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
			print_normal:
			display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, cbuf, color);
			break;
	}
	OS_DEBUG_EXIT();
}

static void ui_keyboard_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_keyboard_render);
	uint8 i,j,c;
	uint16 cx,cy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 bw = w / 10;
	uint16 bh = h / 4;
	ui_keybtn * keyset = g_ba_keyset_keys[((ui_keyboard *)obj)->idx & 0x07];
	ui_keybtn * iterator = keyset;
	uint8 index = 0;
	if(obj->state & UI_STATE_INIT) {
		while(iterator->code != NULL) {
			ui_keyboard_button_render(display, x + (iterator->x * bw), y + (iterator->y * bh), iterator->w * bw, iterator->h * bh, obj->state, iterator->code);
			iterator++;
			index++;
		}
	}
	iterator = keyset;
	if(obj->state & UI_STATE_KEYDOWN && ((ui_keyboard *)obj)->duration == 0) {
		//if(obj->state & UI_STATE_KEYUP) return;
		if_touch_get(display, &cx, &cy);
		cy = cy - y;
		cx = cx - x;
		index = 0;
		while(iterator->code != NULL) {
			if(cx > (iterator->x * bw) && cx <= ((iterator->x * bw) + (iterator->w * bw))) {
				if(cy > (iterator->y * bh) && cy <= ((iterator->y * bh) + (iterator->h * bh))) {
					((ui_keyboard *)obj)->duration = UI_TEXTBOX_PRESS_DURATION;
					((ui_keyboard *)obj)->last_key = index;
					ui_keyboard_button_render(display, x + (iterator->x * bw), y + (iterator->y * bh), iterator->w * bw, iterator->h * bh, obj->state, iterator->code);
				}
			}
			index++;
			iterator++;
		}	
	}
	//key up event (duration based)
	if(((ui_keyboard *)obj)->duration != 0) {
		((ui_keyboard *)obj)->duration--;
		if(((ui_keyboard *)obj)->duration == 0) {
			//iterator = ((ui_keyboard *)obj)->last_key;
			iterator = &keyset[((ui_keyboard *)obj)->last_key];
			if(iterator != NULL) {
				if(iterator->code != NULL) {
					ui_keyboard_button_render(display, x + (iterator->x * bw), y + (iterator->y * bh), iterator->w * bw, iterator->h * bh, UI_STATE_KEYUP, iterator->code);	
				}
			}
		}
	}
	OS_DEBUG_EXIT();
}

static void ui_textkeybox_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_textkeybox_render);
	uint8 buffer[256];
	uint8 i; size_t j;
	uint16 cx, cy, lx, ly;
	uint16 from, to;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 charline = (w / 8) - 1;			//number of character perline
	uint16 chartotal = strlen((const char *)((ui_textbox *)obj)->content);		//calculate total characters
	//charline -= 6;								//space for done button
	if(obj->state & UI_STATE_INIT) {
		display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, x+3, y+4, w-6, h-8));
		display->draw_rectangle(display, x+2, y+3, w-5, h-6, UI_COLOR_WHITE, 0);
		j = charline;
		display->set_area(display, x+3, y+4, w-6, h-8);
		for(i = 0 ; i < chartotal; i += charline, y += UI_FONT_DEFAULT) {
			//print perline
			if((i + charline) > chartotal) j = chartotal - i;
			//check for masked textbox
			if(((ui_textbox *)obj)->mode & 0x04 && (((ui_textbox *)obj)->mode & 0x80) == 0) memset(buffer, '*', j);
			else memcpy(buffer, (const void *)(((ui_textbox *)obj)->content + i), j);
			buffer[j] = 0;
			display->print_string(display, UI_FONT_DEFAULT, x + 4, y + 4, buffer, ((ui_object *)obj)->forecolor);
		}
	}
	x = ((ui_rect *)obj)->x;
	y = ((ui_rect *)obj)->y;
	if(obj->state & UI_STATE_DESELECTED) {
		obj->state &= ~UI_STATE_ACTIVE;
	}
	if(obj->state & UI_STATE_SELECTED || obj->state & UI_STATE_ACTIVE) {
		for(j =((ui_textbox *)obj)->oldidx;j < ((ui_textbox *)obj)->curidx; j++) {
			cx = x + ((j % charline) * 8) + 4;
			cy = y + ((j / charline) * UI_FONT_DEFAULT) + 4;
			if(((ui_textbox *)obj)->blinked & 0x80) {
				//lx = x + ((((ui_textbox *)obj)->oldidx % charline) * 8) + 4;
				//ly = y + ((((ui_textbox *)obj)->oldidx / charline) * UI_FONT_DEFAULT) + 4;
				display->set_area(display, x+3, y+4, w-6, h-8);
				if(((ui_textbox *)obj)->mode & 0x04) {
					//masked textbox
					if(((ui_textbox *)obj)->duration != 0) {
						buffer[0] = ((ui_textbox *)obj)->content[j];
						((ui_textbox *)obj)->duration--;
					}
					if(((ui_textbox *)obj)->duration == 0) {
						if(((ui_textbox *)obj)->content[j] != 0) buffer[0] = '*';
						else buffer[0] = 0;
					}
				} else {
					//non-masked textbox
					buffer[0] = ((ui_textbox *)obj)->content[j];
					((ui_textbox *)obj)->duration = 0;
				}
				buffer[1] = 0;
				display->print_string(display, UI_FONT_DEFAULT, cx, cy, buffer, ((ui_object *)obj)->forecolor);
			}
		}
		//change current index
		if(((ui_textbox *)obj)->duration == 0) {
			if(((ui_textbox *)obj)->curidx > ((ui_textbox *)obj)->oldidx) { 
				for(j =((ui_textbox *)obj)->oldidx;j < ((ui_textbox *)obj)->curidx; j++) {
					cx = x + ((j % charline) * 8) + 4;
					cy = y + ((j / charline) * UI_FONT_DEFAULT) + 4;
					display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, cx, cy, 8, UI_FONT_DEFAULT));
					if(((ui_textbox *)obj)->mode & 0x04 && (((ui_textbox *)obj)->mode & 0x80) == 0) {
						display->print_string(display, UI_FONT_DEFAULT, cx, cy, (uint8 *)"*", ((ui_object *)obj)->forecolor);
					} else {
						buffer[0] = ((ui_textbox *)obj)->content[j];
						buffer[1] = 0;
						display->print_string(display, UI_FONT_DEFAULT, cx, cy, (uint8 *)buffer, ((ui_object *)obj)->forecolor);
					}
				}
			} else {
				for(j =((ui_textbox *)obj)->curidx;j < ((ui_textbox *)obj)->oldidx+1; j++) {
					cx = x + ((j % charline) * 8) + 4;
					cy = y + ((j / charline) * UI_FONT_DEFAULT) + 4;
					display->fill_area(display, ((ui_object *)obj)->backcolor, display->set_area(display, cx, cy, 8, UI_FONT_DEFAULT));
				}
			}
			//print cursor
			cx = x + (((((ui_textbox *)obj)->curidx) % charline) * 8) + 4;
			cy = y + (((((ui_textbox *)obj)->curidx) / charline) * UI_FONT_DEFAULT) + 4;
			if(((ui_textbox *)obj)->blinked & 0x01) {
				//create cursor
				display->fill_area(display, ((ui_object *)obj)->forecolor, display->set_area(display, cx, cy + 2, 1, UI_FONT_DEFAULT - 4));
				((ui_textbox *)obj)->blinked &= 0xFE;		//clear blink status
			} else {
				((ui_textbox *)obj)->blinked |= 0x01;		//set blink status
			}
			((ui_textbox *)obj)->oldidx = ((ui_textbox *)obj)->curidx;
		}
		obj->state |= UI_STATE_ACTIVE;
	}
	OS_DEBUG_EXIT();
}

static void ui_keyboard_button_click(ui_object * keyboard, void * params) {
	OS_DEBUG_ENTRY(ui_keyboard_button_click);
	uint8 i;
	uint8 c;
	uint16 x = ((ui_rect *)keyboard)->x;
	uint16 y = ((ui_rect *)keyboard)->y;
	uint16 w = ((ui_rect *)keyboard)->w;
	uint16 h = ((ui_rect *)keyboard)->h;
	uint16 bw = w / 10;
	uint16 bh = h / 4;
	uint16 cx, cy;
	ui_keybtn * keyset = g_ba_keyset_keys[((ui_keyboard *)keyboard)->idx & 0x07];
	ui_keybtn * iterator = keyset;
	ui_textbox * target = ((ui_textbox *)keyboard->target);
	uint8 charline = (((ui_rect *)target)->w / 8) - 1;			//number of character perline
	uint16 max_chars = ((((ui_rect *)target)->h / UI_FONT_DEFAULT) * charline) -1;
	if_touch_get(params, &cx, &cy);
	cy = cy - y;
	cx = cx - x;
	//c = keyset[((cy / bh) * 10) + (cx / bw)];
	while(iterator->code != NULL) {
		//ui_keyboard_button_render(display, x + (iterator->x * bw), y + (iterator->y * bh), iterator->w * bw, iterator->h * bh, obj->state, iterator->code);
		if(cx > (iterator->x * bw) && cx <= ((iterator->x * bw) + (iterator->w * bw))) {
			if(cy > (iterator->y * bh) && cy <= ((iterator->y * bh) + (iterator->h * bh))) {
				c = iterator->code;
				goto trigger_key_event;
			}
		}
		iterator++;
	}
	if(iterator->code != NULL) {
trigger_key_event:
		iterator = keyset;
		switch(c) {
			case 0x0E:		//shift in
				((ui_keyboard *)keyboard)->idx = 0;			//uppercase letter
				ui_reinitialize_object(keyboard);
				break;
			case 0x0F:		//shift out
				((ui_keyboard *)keyboard)->idx = 1;			//lowercase letter
				ui_reinitialize_object(keyboard);
				break;
			case 0x1A:		//switch symbol
				((ui_keyboard *)keyboard)->idx = 2;
				ui_reinitialize_object(keyboard);
				break;
			case 0x0C:		//enter
				((ui_textbox *)((ui_object *)target)->target)->curidx =  ((ui_textbox *)target)->curidx;
				strcpy((char *)((ui_textbox *)((ui_object *)target)->target)->content, (const char *)((ui_textbox *)target)->content);
				ui_reinitialize_object((ui_object *)((ui_object *)target)->target);
				ui_pop_screen((gui_handle_p)params);
				break;
			case 0x08:
				if(target != NULL && ((ui_textbox *)target)->curidx != 0) {
					((ui_textbox *)target)->content[--((ui_textbox *)target)->curidx] =0;
					goto execute_textbox_handler;
				}
				break;
			default:
				
				if(target != NULL) {
					//put char onto textbox
					if(((ui_textbox *)target)->curidx >=  max_chars) break;
					if(((ui_textbox *)target)->curidx >= ((ui_textbox *)target)->maxlen) break;
					((ui_textbox *)target)->duration = UI_TEXTBOX_PRESS_DURATION;
					((ui_textbox *)target)->content[((ui_textbox *)target)->curidx++] = c;
					((ui_textbox *)target)->content[((ui_textbox *)target)->curidx] = 0;
					execute_textbox_handler:
					if(((ui_object *)target)->handler != NULL) {
						ui_reinitialize_object((ui_object *)target);
					}
				}
				break;
		}
	}
	OS_DEBUG_EXIT();
}

static void ui_keyboard_unmask_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(ui_keyboard_unmask_click);
	ui_textbox * target = ((ui_textbox *)obj->target);
	if(target != NULL) {
		if(target->mode & 0x04) {
			target->mode &= ~0x04;		//char visible
			ui_set_text((ui_text *)obj, (uint8 *)"Hide");
		} else {
			target->mode |= 0x04;		//char masked
			ui_set_text((ui_text *)obj, (uint8 *)"Show");
		}
		ui_reinitialize_object(obj);						//re-render button
		ui_reinitialize_object((ui_object *)target);		//re-render target textbox
	}
	OS_DEBUG_EXIT();
}

static void ui_keyboard_submit_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(ui_keyboard_submit_click);
	ui_textbox * target = ((ui_textbox *)obj->target);
	((ui_textbox *)((ui_object *)target)->target)->curidx =  ((ui_textbox *)target)->curidx;
	strcpy((char *)((ui_textbox *)((ui_object *)target)->target)->content, (const char *)((ui_textbox *)target)->content);
	ui_reinitialize_object((ui_object *)((ui_object *)target)->target);
	ui_pop_screen((gui_handle_p)params);
	OS_DEBUG_EXIT();
}

ui_object * ui_keyboard_create(ui_object * target, uint16 x, uint16 y, uint16 w, uint16 h, uint8 keyset) {
	ui_keyboard * obj = (ui_keyboard *)ui_create_object(sizeof(ui_keyboard), UI_TYPE_LABEL | UI_ALIGN_FLOAT) ;
	((ui_object *)obj)->render = ui_keyboard_render;
	((ui_object *)obj)->rect.x = x;
	((ui_object *)obj)->rect.y = y;
	((ui_object *)obj)->rect.h = h;
	((ui_object *)obj)->rect.w = w;
	obj->idx = keyset;
	((ui_object *)obj)->target = target;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_BLACK;
	((ui_object *)obj)->handler = ui_keyboard_button_click;
	return (ui_object *)obj;
}

ui_object * ui_textkeybox_create(ui_textbox * target, uint16 width, uint16 height) {
	ui_textkeybox * obj = (ui_textkeybox *)ui_create_object(sizeof(ui_textkeybox) + target->maxlen, UI_TYPE_SELECTABLE | UI_ALIGN_GRID) ;
	memcpy(obj, target, sizeof(ui_textbox) + target->maxlen);
	((ui_object *)obj)->rect.w = width;
	((ui_object *)obj)->rect.h = height;
	((ui_object *)obj)->type = UI_TYPE_SELECTABLE | UI_TYPE_VISIBLE | UI_ALIGN_GRID;
	((ui_object *)obj)->parent = NULL;
	((ui_object *)obj)->sibling = NULL;
	((ui_object *)obj)->child = NULL;
	((ui_object *)obj)->state = UI_STATE_INIT;
	((ui_object *)obj)->render = ui_textkeybox_render;
	((ui_object *)obj)->handler = NULL;
	((ui_object *)obj)->target = target;
	((ui_textbox *)obj)->duration = 0;
	return (ui_object *)obj;
}

void ui_keyboard_show(gui_handle_p display, ui_textbox * textbox, uint8 keyset) {
	OS_DEBUG_ENTRY(ui_keyboard_show);
	uint8 i;
	ui_object * keyboard;
	ui_object * textkeybox;
	ui_object * submit;
	ui_object * unmask;
	ui_object * screen;
	screen = ui_push_screen(display, NULL);
	if(screen != NULL) {
		if(textbox->mode & 0x04) {
			//password/PIN textbox
			textkeybox = ui_textkeybox_create(textbox, ((ui_object *)screen)->rect.w - 96, 60);
			if(textkeybox != NULL) {
				ui_add_body(display, textkeybox);
			}
			unmask = ui_button_create(UI_COLOR_WHITE, (uint8 *)"Show", 0, ui_keyboard_unmask_click);
			if(unmask != NULL) {
				unmask->target = textkeybox;
				((ui_object *)unmask)->type = UI_TYPE_BUTTON | UI_ALIGN_GRID | UI_TYPE_VISIBLE;
				//float position is absolute relative to screen
				ui_set_size(unmask, 48, 60);
				ui_add_body(display, unmask);
			}
		} else {
			//normal textbox
			textkeybox = ui_textkeybox_create(textbox, ((ui_object *)screen)->rect.w - 48, 60);
			if(textkeybox != NULL) {
				ui_add_body(display, textkeybox);
			}
		}
		submit = ui_button_create(UI_COLOR_WHITE, (uint8 *)"Done", 0, ui_keyboard_submit_click);
		if(submit != NULL) {
			submit->target = textkeybox;
			((ui_object *)submit)->type = UI_TYPE_BUTTON | UI_ALIGN_GRID | UI_TYPE_VISIBLE;
			//float position is absolute relative to screen
			ui_set_size(submit, 48, 60);
			ui_button_set_style(submit, UI_BUTTON_STYLE_BLUE);
			ui_add_body(display, submit);
		}
		keyboard = ui_keyboard_create(textkeybox, 60, 84, display->width - 60, display->height - 84, keyset);
		memcpy(keyboard, display->body, sizeof(ui_rect));
		((ui_rect *)keyboard)->y += 84;
		((ui_rect *)keyboard)->h -= 84;
		if(keyboard != NULL) ui_add_body(display, keyboard);
		ui_set_selected_object(display, textkeybox);
	}
	OS_DEBUG_EXIT();
}