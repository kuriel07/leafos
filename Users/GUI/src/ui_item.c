#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_item.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\inc\ui_button.h"

void ui_item_text_render(ui_object * obj, gui_handle_p display, uint8 stat) {
	uint16 nc, wtxt, xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 buffer[64];
	uint8 numline, j, k, c;
	uint8 max_text;
	uint8 * ptr;
	uint8 * dptr;
	uint8 * lines[3];
	if(stat & UI_ISTAT_ICON_RENDERED) xx = (((ui_rect *)obj)->h + 4);
	else xx = 4;		//x = width - text_width / 2;
	yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
	if(w > 64) {
		display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
	} else {
		max_text = w / UI_FONT_SMALL;
		ptr = (uint8 *)((ui_text *)obj)->text;
		dptr = buffer;
		for(numline=0;numline<3;numline++) {
			lines[numline] = dptr;
			for(nc=0;nc<max_text;nc++) {
				c = *ptr++;
				if(c == ' ') break;
				if(c == '\n') break;
				if(c == 0) break;
				*dptr++ = c;
			}
			*dptr++ = 0;		//add eos
			if(c == 0) {		//if source not eos, increment source pointer
				numline++; 
				break;
			}	
		}
		yy = (h - (UI_FONT_SMALL * numline)) / 2;
		if(stat & UI_ISTAT_ICON_RENDERED) yy = h - ((UI_FONT_SMALL * numline) + 2);
		for(k=0;k<numline;k++, yy += UI_FONT_SMALL) {
			nc = strlen((const char *)lines[k]);
			wtxt = nc * 8;
			xx = ((w - wtxt) >> 1);		//x = width - text_width / 2;
			display->print_string(display, UI_FONT_SMALL, x + xx, y + yy, (uint8 *)lines[k], UI_COLOR_WHITE);
		}
	}
}

uint8 ui_item_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_item_render);
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 yy, xx, hh, ww;
	uint32 fcolor;
	uint32 bcolor = UI_COLOR_RGB(61,63,71);
	ui_image * thumbnail = NULL;
	uint8 c;
	uint16 halfh = (((ui_rect *)obj)->h / 2);
	ui_object frame;
	uint8 ret = 0;
	if(obj->parent != NULL) {
		bcolor = obj->parent->backcolor;
	}
	if(obj->type & UI_TYPE_GRIDITEM) {
		thumbnail = ((ui_item *)obj)->thumbnail;
	}
	//generate background first
	if(obj->state & UI_STATE_KEYDOWN) {
		display->fill_area(display, UI_COLOR_QUEEN_BLUE, display->set_area(display, x, y, w, h));
		obj->backcolor = UI_COLOR_QUEEN_BLUE;
	} else {
		display->fill_area(display, bcolor, display->set_area(display, x, y, w, h));
		obj->backcolor = bcolor;
	}
	if(thumbnail != NULL) {
		if(w >= thumbnail->base.size.w && h >= thumbnail->base.size.h && h >= 48) {
			xx = (w - thumbnail->base.size.w)/ 2;
			yy = 2;
			memcpy(&frame, obj, sizeof(ui_object));
			((ui_rect *)&frame)->w = thumbnail->base.size.w;
			((ui_rect *)&frame)->h = thumbnail->base.size.h;
			((ui_rect *)&frame)->x += xx;
			((ui_rect *)&frame)->y += yy;
			ui_fill_image(display, (ui_rect *)&frame, thumbnail);
			ret |= UI_ISTAT_ICON_RENDERED;
		}
	} else if(((ui_object *)obj)->image.buffer != NULL) {
		memcpy(&frame, obj, sizeof(ui_object));
		((ui_rect *)&frame)->w = ((ui_rect *)&frame)->h;
		ui_resource_render(display, (ui_rect *)&frame, &((ui_object *)obj)->image, UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		ret |= UI_ISTAT_ICON_RENDERED;
	}
	if(w > 64) {
		//display separator border (top)
		display->fill_area(display, UI_COLOR_RGB(49,51,59), display->set_area(display, x, y, w, 1));
		//display separator border (bottom)
		display->fill_area(display, UI_COLOR_RGB(79,81,89), display->set_area(display, x, y + h - 1, w, 1));
		ret |= UI_ISTAT_BORDER_RENDERED;
	}
	OS_DEBUG_EXIT();
	return ret;
}

static void ui_listitem_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_listitem_render);
	uint16 nc, wtxt, xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 stat;
	if(obj->state != 0) {
		stat = ui_item_render(obj, display);
		display->set_area(display, x, y, w, h);
		ui_item_text_render(obj, display, stat);
	}
	OS_DEBUG_EXIT();
}

static void ui_select_item_draw_button(gui_handle_p display, ui_object * obj, uint8 state, uint8 * text) {
	OS_DEBUG_ENTRY(ui_select_item_draw_button);
	uint16 nc, wtxt, xx, yy;
	nc = strlen((const char *)text);
	wtxt = nc * 8;
	xx = ((((ui_rect *)obj)->w - wtxt) >> 1);		//x = width - text_width / 2;
	yy = ((((ui_rect *)obj)->h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
	ui_draw_button(display, (ui_rect *)obj, state);
	display->print_string(display, UI_FONT_DEFAULT, ((ui_rect *)obj)->x + xx, ((ui_rect *)obj)->y + yy, (uint8 *)text, UI_COLOR_WHITE);
	OS_DEBUG_EXIT();
}

static void ui_orbriver_item_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_orbriver_item_render);
	uint16 nc, wtxt, xx, yy;
	uint8 tbuf[12];
	ui_rect temp;
	uint16 cx, cy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	ui_object detail;
	uint8 stat;
	if(obj->state != 0) {
		stat = ui_item_render(obj, display);
		display->set_area(display, x, y, w, h);
		display->draw_line(display, x, y, x+w, y, UI_COLOR_BLACK);
		display->draw_line(display, x, y+h-1, x+w, y+h-1, UI_COLOR_WHITE);
		xx = 4;		//x = width - text_width / 2;
		yy = 0;
		if(w > 64) {
			display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
			sprintf((char *)tbuf, "Size : %0.1f KB", (float)((ui_orbriver_item *)obj)->size / 1024.0);
			display->print_string(display, UI_FONT_SMALL, x + xx + 2, y + yy + 18, tbuf, ((ui_object *)obj)->forecolor);
			memcpy(&detail, obj, sizeof(ui_object));
			((ui_rect *)&detail)->x = x + w - 74;
			((ui_rect *)&detail)->y = y + 2;
			((ui_rect *)&detail)->w = 72;
			((ui_rect *)&detail)->h = h - 4;
			ui_select_item_draw_button(display, &detail, UI_BUTTON_STYLE_NORMAL | UI_BUTTON_STATE_NONE, (uint8 *)"Detail");
			if(obj->state & UI_STATE_KEYDOWN) {
				//if(obj->state & UI_STATE_KEYUP) return;
				if_touch_get(display, &cx, &cy);
				temp.x = cx;
				temp.y = cy;
				temp.w = 1;
				temp.h = 1;
				((ui_orbriver_item *)obj)->event = UI_ITEM_EVENT_CLICK;
				if(ui_check_overlapped((ui_rect *)&detail, (ui_rect *)&temp)) {
					ui_select_item_draw_button(display, &detail, UI_BUTTON_STYLE_BLUE | UI_BUTTON_STATE_PRESSED, (uint8 *)"Detail");
					((ui_orbriver_item *)obj)->event = UI_ITEM_EVENT_DETAIL;
				}
			}
		} else {
			ui_item_text_render(obj, display, stat);
		}
	}
	OS_DEBUG_EXIT();
}

static void ui_select_item_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_select_item_render);
	uint16 cx,cy;
	uint16 nc, wtxt, xx, yy;
	uint8 state = UI_BUTTON_STATE_NONE;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	ui_object temp; //= { x, y + 1, h-2, h-2};
	char * text;
	uint8 stat;
	memcpy(&temp, obj, sizeof(ui_object));
	((ui_rect *)&temp)->y = y + 1;
	((ui_rect *)&temp)->w = h-2;
	((ui_rect *)&temp)->h = h-2;
	ui_select_item_list * cur = &((ui_select_item *)obj)->list[((ui_select_item *)obj)->index];
	if(((ui_object *)obj)->image.buffer != NULL) xx = (((ui_rect *)obj)->h + 4);
	else xx = 4;		//x = width - text_width / 2;
	yy = ((h - UI_FONT_DEFAULT) >> 1);		//y = height - text_height /2;
	if(obj->state != 0) {
		stat = ui_item_render(obj, display);
		//display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
		if(w > 64) {
			display->print_string(display, UI_FONT_DEFAULT, x + xx, y + yy, (uint8 *)cur->text, ((ui_object *)obj)->forecolor);
			display->fill_area(display, UI_COLOR_RGB(79,81,89), display->set_area(display, x, y + h-2, w, 1));
			display->fill_area(display, UI_COLOR_RGB(60,62,70), display->set_area(display, x, y + h-1, w, 1));
			text = "-";
			((ui_rect *)&temp)->x = x + w-(h * 2);
			ui_select_item_draw_button(display, &temp, UI_BUTTON_STATE_NONE, (uint8 *)text);
			text = "+";
			((ui_rect *)&temp)->x = x + w-h;
			ui_select_item_draw_button(display, &temp, UI_BUTTON_STATE_NONE, (uint8 *)text);
			if(obj->state & UI_STATE_KEYDOWN) {
				//if(obj->state & UI_STATE_KEYUP) return;
				if_touch_get(display, &cx, &cy);
				cy = cy - y;
				cx = cx - x;
				if (cx >= (w-(h*2)) && cx < (w-h)) {
					//down operation
					text = "-";
					((ui_rect *)&temp)->x = x + w-(h * 2);
					if(((ui_select_item *)obj)->index != 0) ((ui_select_item *)obj)->index--;
				} else if(cx >= (w-h) && cx < w) {
					//up operation
					text = "+";
					((ui_rect *)&temp)->x = x + (w-h);
					if(((ui_select_item *)obj)->index < (((ui_select_item *)obj)->total - 1)) ((ui_select_item *)obj)->index++;
				} else 
					return;			//do nothing
				ui_select_item_draw_button(display, &temp, UI_BUTTON_STATE_PRESSED, (uint8 *)text);
				//set current text
			}
		} else {
			strcpy(((ui_text *)obj)->text, (const char *)(uint8 *)cur->text); 
			ui_item_text_render(obj, display, stat);
		}
	}
	if(((ui_select_item *)obj)->duration != 0) {
		((ui_select_item *)obj)->duration--;
		if(((ui_select_item *)obj)->duration == 0) {
			ui_reinitialize_object(obj);		//re-init object
		}
	}
	OS_DEBUG_EXIT();
}

static void ui_show_detail_done_click(ui_object * obj, void * params) {
	ui_pop_screen((gui_handle_p)params);
}

void ui_item_show_detail(gui_handle_p display, ui_orbriver_item * item) {
	ui_object * panel = ui_push_screen(display, NULL);
	ui_object * label;
	ui_object * btn;
	if(panel != NULL) {
		label = ui_label_create(UI_COLOR_WHITE, 0, UI_FONT_DEFAULT, item->desc);
		btn = ui_button_create(UI_COLOR_WHITE, (uint8 *)"Done", 8, ui_show_detail_done_click);
		ui_add_body(display, label);
		ui_add_body(display, btn);
	}
}

ui_object * ui_orbriver_item_create(uchar * text, uint8 * oid, uint16 size, uint32 nod, uint16 bnum, uint8 * hash, uint8 * desc, void (* click)(ui_object *, void * params)) {
	//ui_rect rect = { 100, 2, 72, 22 };
	//ui_object * btn;
	ui_orbriver_item * obj = (ui_orbriver_item *)ui_create_item(sizeof(ui_orbriver_item), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, (char *)text) ;
	if(oid != NULL) strcpy((char *)obj->oid, (const char *)oid);			//orblet identifier
	obj->nod = nod;
	obj->size = size;			//orblet size
	if(hash != NULL) {
		memcpy(obj->hash, hash, SHARD_HASH_SIZE);			//hash
	}
	obj->bnum = bnum;																	//build number
	obj->event = UI_ITEM_EVENT_CLICK;												//default event/detail
	if(strlen((const char *)desc) != 0) {
		strncpy((char *)obj->desc, (const char *)desc, SHARD_MAX_DESC);		//copy descriptor
	} else {
		strcpy((char *)obj->desc, "no description available");
	}
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_WHITE;
	((ui_object *)obj)->render = ui_orbriver_item_render;
	((ui_object *)obj)->handler = click;
	((ui_item *)obj)->id = 0;
	return (ui_object *)obj;
}

ui_object * ui_item_create(uchar * text, uint8 * bitmap, uint32 bmpsize, uint8 id, void (* click)(ui_object *, void *)) {
	ui_item * obj = (ui_item *)ui_create_item(sizeof(ui_item), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, (char *)text) ;
	obj->id = id;
	((ui_object *)obj)->rect.h = 48;
	((ui_object *)obj)->image.buffer = bitmap;
	((ui_object *)obj)->image.size = bmpsize;
	((ui_object *)obj)->render = ui_listitem_render;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_WHITE;
	((ui_object *)obj)->handler = click;
	return (ui_object *)obj;
}

ui_object * ui_app_item_create(uchar * text, uint8 aidlen, uint8 * aid, void (* click)(ui_object *, void * params))  {
	ui_app_item * obj = (ui_app_item *)ui_create_item(sizeof(ui_app_item), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, (char *)text) ;
	((ui_object *)obj)->rect.h = 48;
	((ui_object *)obj)->render = ui_listitem_render;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_WHITE;
	((ui_object *)obj)->handler = click;
	((ui_object *)obj)->image.buffer = NULL;
	((ui_object *)obj)->image.size = 0;
	((ui_item *)obj)->id = 0;
	obj->aidlen = aidlen;
	memcpy(obj->aid, aid, aidlen);
	return (ui_object *)obj;
}

ui_object * ui_select_item_create(uchar * text, uint8 index, uint8 total, ui_select_item_list * list, void (* click)(ui_object *, void * params)) {
	ui_select_item * obj = (ui_select_item *)ui_create_item(sizeof(ui_select_item), UI_TYPE_BUTTON | UI_ALIGN_VERTICAL, (char *)text) ;
	((ui_object *)obj)->rect.h = 48;
	((ui_object *)obj)->render = ui_select_item_render;
	((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(61,63,71);;
	((ui_object *)obj)->handler = click;
	obj->index = index;
	obj->total = total;
	obj->list = list;
	return (ui_object *)obj;
}

void * ui_create_item(size_t objsize, uint32 mode, char * text) {
	ui_item * obj = (ui_item *)ui_create_text(objsize, UI_TYPE_GRIDITEM | mode, text);
	if(obj == NULL) return NULL;
	obj->thumbnail = NULL;
	obj->id = 0;
	return obj;
}