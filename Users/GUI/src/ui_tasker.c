#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_core.h"
#include "..\inc\ui_tasker.h"
#include "..\inc\ui_resources.h"
#include "..\inc\ui_window.h"
#include "..\inc\ui_scroll.h"
#include "..\inc\ui_item.h"

static void ui_tasker_render(ui_object * obj, gui_handle_p display) {
	uint16 nc, wtxt, xx, yy;
	uint16 ww, hh, num_pal;
	uint8 i, j, c;
	uint16 offset;
	uint32 color;
	ui_object temp;
	
	uint16 x = display->width - ((ui_object *)obj)->rect.w;
	uint16 y = 0;
	uint16 w = ((ui_object *)obj)->rect.w;
	uint16 h = ((ui_object *)obj)->rect.h;
	if(obj->state & UI_STATE_INIT) {
		((ui_rect *)obj)->x = x;
		((ui_rect *)obj)->y = y;
		((ui_rect *)obj)->w = w;
		((ui_rect *)obj)->h = h;
		((ui_object *)obj)->rect.x = x;
		((ui_object *)obj)->rect.y = y;
		display->set_area(display, x, y, w, h);
		//calculate and print image
		memcpy(&temp, obj, sizeof(ui_object));
		temp.backcolor = (obj->parent)->backcolor;		//use parent back color
		if(((ui_toggle *)obj)->state & UI_TOGGLE_ON) {
			ui_image_render(display, (ui_rect *)&temp, (uint8 *)image_png_tasker_x24, sizeof(image_png_tasker_x24), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		} else {
			ui_image_render(display, (ui_rect *)&temp, (uint8 *)image_png_tasker_w24, sizeof(image_png_tasker_w24), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		}
		
	}
}

static void ui_item_window_click(ui_object * obj, void * param) {
	ui_object * tskBtn;
	ui_object * window = ui_get_object_by_name(param, "taskList");
	ui_remove_screen_unsafe(param, obj->target);
	ui_push_screen(param, obj->target);
	if(window != NULL) {
		ui_window_close(param, (ui_window *)window);
	}
	tskBtn =  ui_get_object_by_name(param, "taskBtn");
	if(tskBtn != NULL) {
		ui_set_toggle_state(tskBtn, UI_TOGGLE_OFF);
	}
}

static void ui_tasker_click(ui_object * obj, void * param) {
	ui_window * window;
	gui_handle_p display = (gui_handle_p)param;
	ui_object * iterator;
	uint8 index = 0;
	ui_object * item;
	ui_object * tskBtn;
	window = (ui_window *)ui_get_object_by_name(param, "taskList");
	if(window != NULL) {
		ui_window_close(param, (ui_window *)window);
		tskBtn =  ui_get_object_by_name(param, "taskBtn");
		if(tskBtn != NULL) {
			ui_set_toggle_state(tskBtn, UI_TOGGLE_OFF);
		}
		return;
	} else {
		tskBtn =  ui_get_object_by_name(param, "taskBtn");
		if(tskBtn != NULL) {
			ui_set_toggle_state(tskBtn, UI_TOGGLE_ON);
		}
		window =  ui_window_show(param, (uint8 *)"Switch", 0, 24); 
		if(window != NULL) {
			ui_set_object_name((ui_object *)window, "taskList");
			//while(iterator != NULL) {
			for(index=0;index < display->stack_index;index++) {
				iterator = display->stack[index];
				if(iterator == NULL) continue;
				if(iterator == (ui_object *)window) continue;
				if((iterator->type & UI_TYPE_PANEL) == 0) continue;
				item = ui_item_create((uint8 *)((ui_text *)iterator)->text, (uint8 *)image_png_window24, sizeof(image_png_window24), 0, ui_item_window_click);
				if(item != NULL) {
					ui_set_target(item, iterator);
					ui_list_add(window->panel, (ui_object *)item);
					//ui_set_image(item, (uint8 *)image_png_window24, sizeof(image_png_window24));
					((ui_item *)item)->thumbnail = ((ui_panel *)iterator)->thumbnail;
				}
				//iterator = iterator->sibling;
			}
		}
	}
}

ui_object * ui_tasker_create() {
	ui_tasker * obj = (ui_tasker *)ui_create_text(sizeof(ui_toggle), UI_TYPE_BUTTON | UI_ALIGN_HORIZONTAL, (char *)"") ;
	if(obj != NULL) {
		((ui_object *)obj)->rect.w = 24;
		((ui_object *)obj)->rect.h = 24;
		((ui_object *)obj)->forecolor = UI_COLOR_WHITE;
		((ui_object *)obj)->backcolor = UI_COLOR_SLATE_GREY;
		((ui_object *)obj)->render = ui_tasker_render;
		((ui_object *)obj)->handler = ui_tasker_click;
		ui_set_object_name((ui_object *)obj, "taskBtn");
	}
	return (ui_object *)obj;
}