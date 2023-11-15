#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\inc\ui_signal.h"
#include "..\inc\ui_panel.h"
#include "..\inc\ui_button.h"
#include "..\inc\ui_label.h"
#include "..\inc\ui_textbox.h"
#include "..\inc\ui_scroll.h"
#include "..\inc\ui_resources.h"
#include "..\..\interfaces\inc\if_apis.h"

#define UI_SCROLL_STATE_UP				1
#define UI_SCROLL_STATE_DOWN			2

static void ui_list_view_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_list_view_render);
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 scroll_w = 24;
	uint16 halfh = h/2;
	ui_object * list_area = NULL;
	ui_object * view_btn = NULL;
	ui_object * next_btn = NULL;
	ui_object *	prev_btn = NULL;
	if(obj->state != 0) {
		//set list_area rect
		list_area = (ui_object * )((ui_list_view *)obj)->list_area;
		ui_set_position(list_area, x, y);
		ui_set_size(list_area, w - scroll_w, h);
		//list_area->render(list_area, display);
		//set view_btn rect
		view_btn = (ui_object * )((ui_list_view *)obj)->view_btn;
		ui_set_position(view_btn, x + (w - scroll_w), y);
		ui_set_size(view_btn, scroll_w, 24);
		//set next_btn rect
		next_btn = (ui_object * )((ui_list_view *)obj)->next_btn;
		ui_set_position(next_btn, x + (w - scroll_w), y + 24);
		ui_set_size(next_btn, scroll_w, 24);
		//next_btn->render(next_btn, display);
		//set prev_btn rect
		prev_btn = (ui_object * )((ui_list_view *)obj)->prev_btn;
		ui_set_position(prev_btn, x + (w - scroll_w), y + 48);
		ui_set_size(prev_btn, scroll_w, 24);
		//prev_btn->render(prev_btn, display);
		//fill scroll area
		display->fill_area(display, obj->backcolor, display->set_area(display, x + scroll_w, y, w - scroll_w, h));
	}
	OS_DEBUG_EXIT();
}

static void ui_list2_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_list2_render);
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 yy, xx, hh, ww;
	uint32 fcolor;
	uint8 c;
	uint8 cr;
	uint16 curIdx = 0;
	uint16 curTotal = 0;
	uint16 halfh = h/2;
	uint16 cx, cy;
	ui_object * prev = NULL;
	ui_object * temp;
	uint16 page_length;
	uint16 item_height;
	uint16 item_width = ((ui_rect *)obj)->w;
	uint16 columns, rows;
	ui_object * iterator = ((ui_object *)obj)->child;
	uint16 startIdx = ((ui_list_scroll *)obj)->cur_index;
	switch( ((ui_list_scroll *)obj)->mode) {
		case UI_LIST_MODE_LIST:
			item_height =  (((ui_rect *)obj)->h / 5);
			page_length = ((ui_list_scroll *)obj)->page_length = 5;
			if((h / 5) < 24) {
				page_length = ((ui_list_scroll *)obj)->page_length = h / 24;
				item_height = 24;
			} else if((h / 5) > 48) {
				page_length = ((ui_list_scroll *)obj)->page_length = h / 48;
				item_height = 48;
			}
			break;
		case UI_LIST_MODE_GRID:
			item_width = ((ui_rect *)obj)->w / 4;
			item_height = ((ui_rect *)obj)->w / 5;
			if(item_width < 48) item_width = 48;
			if(item_width > 64) {
				item_width = 64;
			}
			if(item_height < 48) item_height = 48;
			if(item_height > 64) {
				item_height = 64;
			}
			columns = ((ui_rect *)obj)->w / item_width;
			rows = ((ui_rect *)obj)->h / item_height;
			((ui_list_scroll *)obj)->page_length = page_length = columns * rows;
			break;
	}
	//calculate start list offset
#if 0
	if((((ui_list_scroll *)obj)->scroll_state & 0x03) != 0) {
		iterator = ((ui_object *)obj)->child;
		//calculate total list item
		while(iterator != NULL) {
			iterator->type &= ~UI_TYPE_VISIBLE;			//hide object
			iterator = iterator->sibling;
			curTotal ++;
		}
		switch(((ui_list_scroll *)obj)->scroll_state) {
			case UI_SCROLL_STATE_UP:
				if(((ui_list_scroll *)obj)->cur_index < page_length) startIdx = ((ui_list_scroll *)obj)->cur_index = 0;
				else startIdx = ((ui_list_scroll *)obj)->cur_index - page_length;
				((ui_list_scroll *)obj)->scroll_state = 0;
				goto start_list_item;
			case UI_SCROLL_STATE_DOWN:
				if((((ui_list_scroll *)obj)->cur_index + page_length) >= curTotal && (curTotal > page_length))  startIdx = curTotal - page_length;
				else {
					if(curTotal <= page_length) startIdx = 0;
					else startIdx = ((ui_list_scroll *)obj)->cur_index + page_length;
				}
				((ui_list_scroll *)obj)->scroll_state = 0;
				goto start_list_item;
			default: break;
		}
	}
#endif
	
	if(obj->state != 0) {
		//calculate item area
		start_list_item:
		iterator = ((ui_object *)obj)->child;
#if 0
		prev = NULL;
		//hide list object
		while(iterator != NULL) {
			//skip iterator if state == DETACH
			if(iterator->lcs == UI_LCS_DETACHED) {
				temp = iterator->sibling;
				//relink
				if(prev == NULL) iterator->parent->child = temp;
				else prev->sibling = temp;
				ui_delete_object(iterator);
				iterator = temp;
				continue;
			}
			prev = iterator;
			iterator->type &= ~UI_TYPE_VISIBLE;			//hide object
			iterator = iterator->sibling;
		}
#else
		//hide list object
		while(iterator != NULL) {
			iterator->type &= ~UI_TYPE_VISIBLE;			//hide object
			iterator = iterator->sibling;
		}
#endif
		//update current list offset
		startIdx = ((ui_list_scroll *)obj)->cur_index;
		xx = 0;
		yy = 0;
		curTotal = 0;
		curIdx = 0;
		if_touch_get(display, &cx, &cy);
		//render list item according to their own renderer
		iterator = ((ui_object *)obj)->child;
		switch( ((ui_list_scroll *)obj)->mode) {
			case UI_LIST_MODE_LIST:
				while(iterator != NULL && curTotal < page_length) {
					if(curIdx >= startIdx) {
						iterator->type |= UI_TYPE_VISIBLE;
						((ui_rect *)iterator)->x = x +xx;
						((ui_rect *)iterator)->y = y +yy;
						((ui_rect *)iterator)->w = item_width;
						((ui_rect *)iterator)->h = item_height;
						memcpy(&iterator->rect, iterator, sizeof(ui_rect));
						if(ui_check_intersect((ui_rect *)iterator, cx, cy)) {
							iterator->state = obj->state;
						}
						if(iterator->state != 0) {
							((ui_object *)iterator)->render(iterator, display);
							iterator->state = 0;
						}
						yy += ((ui_rect *)iterator)->h;
						//switch to next sibling
						curTotal++;
					}
					iterator = iterator->sibling;
					curIdx ++;
				}
				//fill empty items
				display->fill_area(display, obj->backcolor, display->set_area(display, x, y + yy, w, h - yy));
				break;
			case UI_LIST_MODE_GRID:
				//fill empty items
				while(iterator != NULL && curTotal < page_length) {
					if(curIdx >= startIdx) {
						iterator->type |= UI_TYPE_VISIBLE;
						if(iterator->type & UI_TYPE_BUTTON) {
							((ui_rect *)iterator)->x = x + xx;
							((ui_rect *)iterator)->y = y + yy;
							((ui_rect *)iterator)->w = item_width;
							((ui_rect *)iterator)->h = item_height;
							memcpy(&iterator->rect, iterator, sizeof(ui_rect));
							if(ui_check_intersect((ui_rect *)iterator, cx, cy)) {
								iterator->state = obj->state;
							}
							if(iterator->state != 0) {
								((ui_object *)iterator)->render(iterator, display);
								iterator->state = 0;
							}
							//calculate next grid
							xx += item_width;
							if((x + xx + item_width) > (x + w)) {
								display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y + yy, w - xx, item_height));
								xx = 0;		//reset xx
								yy += item_height;		//increase yy
							}
						} else {
							//fill empty space if any
							if(xx != 0) {
								display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y + yy, w - xx, item_height));
								yy += item_height;
								xx = 0;
							}
							//fill background first 
							display->fill_area(display, obj->backcolor, display->set_area(display, x, y + yy, w, ((ui_rect *)iterator)->h));
							((ui_rect *)iterator)->x = x + xx;
							((ui_rect *)iterator)->y = y + yy;
							((ui_rect *)iterator)->w = item_width;
							((ui_rect *)iterator)->h = item_height;
							memcpy(&iterator->rect, iterator, sizeof(ui_rect));
							if(iterator->state != 0) {
								((ui_object *)iterator)->render(iterator, display);
								iterator->state = 0;
							}
							yy += ((ui_rect *)iterator)->h;
						}
						//switch to next sibling
						curTotal++;
					}					
					iterator = iterator->sibling;
					curIdx ++;
				}
				if(xx != 0) {
					display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y + yy, w - xx, item_height));
					yy += item_height;
					xx = 0;
				}
				if(yy != h) {
					display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y + yy, w, h - yy));
				}
				break;
		}					//up button
	}
	OS_DEBUG_EXIT();
}

void ui_scroll2_click(ui_object * obj, void * display) {
	OS_DEBUG_ENTRY(ui_scroll2_click);
	uint16 cx, cy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 curIdx = 0;
	ui_object * iterator = ((ui_object *)obj)->child;
	if_touch_get(display, &cx, &cy);
	
	while(iterator != NULL) {
		if(curIdx == ((ui_list_scroll *)obj)->cur_index) break;
		iterator = iterator->sibling;
		curIdx ++;
	}
	while(iterator != NULL) {
		if(cy > ((ui_rect *)iterator)->y && cy <= (((ui_rect *)iterator)->y  + ((ui_rect *)iterator)->h) &&
		cx > ((ui_rect *)iterator)->x && cx <= (((ui_rect *)iterator)->x  + ((ui_rect *)iterator)->w)) {	
			if(iterator->handler != NULL) iterator->handler(iterator, display);
			((ui_container *)obj)->selected = iterator;
			ui_reinitialize_object(obj);
			ui_reinitialize_object(iterator);
			break;
		}
		iterator = iterator->sibling;
	}
	OS_DEBUG_EXIT();
}

static void ui_list_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_list_render);
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 yy, xx, hh, ww;
	uint32 fcolor;
	uint8 c;
	uint8 cr;
	uint16 curIdx = 0;
	uint16 curTotal = 0;
	uint16 halfh = h/2;
	uint16 cx, cy;
	ui_object * prev = NULL;
	ui_object * temp;
	ui_object * iterator = ((ui_object *)obj)->child;
	uint16 startIdx = ((ui_list_scroll *)obj)->cur_index;
	uint16 scroll_w = (w * 15) / 100;
	//calculate start list offset
	if((((ui_list_scroll *)obj)->scroll_state & 0x03) != 0) {
		iterator = ((ui_object *)obj)->child;
		//calculate total list item
		while(iterator != NULL) {
			iterator->type &= ~UI_TYPE_VISIBLE;			//hide object
			iterator = iterator->sibling;
			curTotal ++;
		}
		switch(((ui_list_scroll *)obj)->scroll_state) {
			case UI_SCROLL_STATE_UP:
				if(((ui_list_scroll *)obj)->cur_index < 5) startIdx = ((ui_list_scroll *)obj)->cur_index = 0;
				else startIdx = ((ui_list_scroll *)obj)->cur_index - 5;
				((ui_list_scroll *)obj)->scroll_state = 0;
				display->fill_area(display, UI_COLOR_QUEEN_BLUE, display->set_area(display, x + w - scroll_w, y, scroll_w, halfh));					//up button
				goto start_list_item;
			case UI_SCROLL_STATE_DOWN:
				if((((ui_list_scroll *)obj)->cur_index + 5) >= curTotal && (curTotal > 5))  startIdx = curTotal - 5;
				else {
					if(curTotal <= 5) startIdx = 0;
					else startIdx = ((ui_list_scroll *)obj)->cur_index + 5;
				}
				display->fill_area(display, UI_COLOR_QUEEN_BLUE, display->set_area(display, x + w - scroll_w, y + h - halfh, scroll_w, halfh));		//down button
				((ui_list_scroll *)obj)->scroll_state = 0;
				goto start_list_item;
			default: break;
		}
	}
	
	if(obj->state != 0) {
		//calculate item area
		memcpy((ui_rect *)&obj->rect, (ui_rect *)obj, sizeof(ui_rect));
		//obj->base.w = obj->base.w - 48;
		start_list_item:
		iterator = ((ui_object *)obj)->child;
#if 0
		prev = NULL;
		//hide list object
		while(iterator != NULL) {
			//skip iterator if state == DETACH
			if(iterator->lcs == UI_LCS_DETACHED) {
				temp = iterator->sibling;
				//relink
				if(prev == NULL) iterator->parent->child = temp;
				else prev->sibling = temp;
				ui_delete_object(iterator);
				iterator = temp;
				continue;
			}
			prev = iterator;
			iterator->type &= ~UI_TYPE_VISIBLE;			//hide object
			iterator = iterator->sibling;
		}
#else
		//hide list object
		while(iterator != NULL) {
			iterator->type &= ~UI_TYPE_VISIBLE;			//hide object
			iterator = iterator->sibling;
		}
		
#endif
		//update current list offset
		((ui_list_scroll *)obj)->cur_index = startIdx;
		xx = 0;
		yy = 0;
		curTotal = 0;
		curIdx = 0;
		if_touch_get(display, &cx, &cy);
		//render list item according to their own renderer
		iterator = ((ui_object *)obj)->child;
		while(iterator != NULL && curTotal < 5) {
			if(curIdx >= startIdx) {
				iterator->type |= UI_TYPE_VISIBLE;
				((ui_rect *)iterator)->x = x +xx;
				((ui_rect *)iterator)->y = y +yy;
				((ui_rect *)iterator)->w = w - scroll_w;
				((ui_rect *)iterator)->h =  (((ui_rect *)obj)->h / 5);
				if(ui_check_intersect((ui_rect *)iterator, cx, cy)) {
					iterator->state = obj->state;
				}
				if(iterator->state != 0) {
					((ui_object *)iterator)->render(iterator, display);
					iterator->state = 0;
				}
				yy += ((ui_rect *)iterator)->h;
				//switch to next sibling
				curTotal++;
			}
			iterator = iterator->sibling;
			curIdx ++;
		}
		//fill empty items
		display->fill_area(display, obj->backcolor, display->set_area(display, x, y + yy, w - scroll_w, h - yy));					//up button
	}
	if((obj->state & UI_STATE_INIT) || (obj->state & UI_STATE_KEYUP)) {
		//only redraw if scroll state is inactive (during initialization
		display->fill_area(display, UI_COLOR_RGB(58,60,66), display->set_area(display, x + w - scroll_w, y, scroll_w, halfh));					//up button
		display->fill_area(display, UI_COLOR_RGB(58,60,66), display->set_area(display, x + w - scroll_w, y + h - halfh, scroll_w, halfh));		//down button
		
		for(c=0,xx=16,yy=0;c<8;c++,xx+=2,yy+=(halfh / 8)) {
			cr = 110 - (c*5);
			//cr = (c * 5) + 70;
			display->fill_area(display, UI_COLOR_RGB(cr,cr,cr), display->set_area(display, x + (w - scroll_w) + (xx / 2), y + yy + (halfh / 32), scroll_w - xx, halfh / 16));					//up button
			display->fill_area(display, UI_COLOR_RGB(cr,cr,cr), display->set_area(display, x + (w - scroll_w) + (xx / 2), y + (h - (halfh / 16)) - yy - (halfh / 32), scroll_w - xx, halfh / 16));					//up button
			//display->fill_area(display, UI_COLOR_RGB(cr,cr,cr), display->set_area(display, x + (w - 48) + (xx / 2), y + (halfh - (halfh / 16)) - yy - (halfh / 32), 48 - xx, halfh / 16));					//up button
			//display->fill_area(display, UI_COLOR_RGB(cr,cr,cr), display->set_area(display, x + (w - 48) + (xx / 2), y + halfh + yy + (halfh / 32), 48 - xx, halfh / 16));					//up button
		}
	}
	OS_DEBUG_EXIT();
}

void ui_scroll_click(ui_object * obj, void * display) {
	OS_DEBUG_ENTRY(ui_scroll_click);
	uint16 cx, cy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj)->w;
	uint16 h = ((ui_rect *)obj)->h;
	uint16 curIdx = 0;
	ui_object * iterator = ((ui_object *)obj)->child;
	if_touch_get(display, &cx, &cy);
	if(cx > (x + (w - 48))) {			//navigation
		if(cy < (y + 60)) {			//up navigation
			((ui_list_scroll *)obj)->scroll_state = UI_SCROLL_STATE_UP;
#if HWCL_ENABLED && LCD_FMC_ENABLED
			ui_set_animation(obj, UI_ANIM_SLIDE_LEFT);
#endif
		}
		if(cy > (y + (h-60))) {		//down navigation
			((ui_list_scroll *)obj)->scroll_state = UI_SCROLL_STATE_DOWN;
#if HWCL_ENABLED && LCD_FMC_ENABLED
			ui_set_animation(obj, UI_ANIM_SLIDE_RIGHT);
#endif
		}
		ui_reinitialize_object(obj);
		while(iterator != NULL) {
			ui_reinitialize_object(iterator);
			iterator = iterator->sibling;
		}
	} else {							//item click
		while(iterator != NULL) {
			if(curIdx == ((ui_list_scroll *)obj)->cur_index) break;
			iterator = iterator->sibling;
			curIdx ++;
		}
		while(iterator != NULL) {
			if(cy > ((ui_rect *)iterator)->y && cy <= (((ui_rect *)iterator)->y  + ((ui_rect *)iterator)->h)) {
				iterator->handler(iterator, display);
				((ui_container *)obj)->selected = iterator;
				ui_reinitialize_object(obj);
				ui_reinitialize_object(iterator);
				break;
			}
			iterator = iterator->sibling;
		}
	}
	OS_DEBUG_EXIT();
}

void ui_list_add(ui_object * list, ui_object * obj) {
	ui_object * iterator;
	ui_object * list_area = (ui_object *)((ui_list_view *)list)->list_area;
	if(list_area->child == NULL) {
		list_area->child = obj;
	} else {
		iterator = list_area->child;
		while(iterator->sibling != NULL) {
			iterator = iterator->sibling;
		}
		iterator->sibling = obj;
	}
	obj->parent = list_area;
	obj->display = list_area->display;
}

static void ui_list_release(void * ptr) {
	ui_object * iterator = ((ui_object *)ptr)->child;
	ui_object * temp;
	while(iterator != NULL) {
		temp = iterator;
		iterator = iterator->sibling;
		//temp->release(temp);
		if(temp->release != NULL) temp->release(temp);
		memset(temp, 0, sizeof(ui_object));
		free(temp);
	}
}


ui_object * ui_list2_create(uint16 x, uint16 y, uint16 w, uint16 h) {
	//ui_list_scroll * obj = (ui_list_scroll *)ui_create_object(sizeof(ui_list_scroll), UI_TYPE_GRIDITEM | UI_ALIGN_GRID | UI_TYPE_CONTAINER) ;
	ui_list_scroll * obj = (ui_list_scroll *)ui_create_object(sizeof(ui_list_scroll), UI_ALIGN_NONE | UI_TYPE_CONTAINER) ;
	((ui_object *)obj)->rect.x = x;
	((ui_object *)obj)->rect.y = y;
	((ui_object *)obj)->rect.w = w;
	((ui_object *)obj)->rect.h = h;
	((ui_object *)obj)->release = ui_list_release;
	((ui_object *)obj)->render = ui_list2_render;
	((ui_object *)obj)->forecolor = UI_COLOR_BLUE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(23,24,36);
	((ui_list_scroll *)obj)->scroll_state = 0;
	((ui_list_scroll *)obj)->cur_index = 0;
	((ui_object *)obj)->handler = ui_scroll2_click;
	//if(click != NULL) ((ui_object *)obj)->handler = click;
	return (ui_object *)obj;
}

void ui_list_clear(gui_handle_p display, ui_list_scroll * list ) {
		ui_clear_child(display, ((ui_object *)list)->child);
}

static void ui_scroll_view_click(ui_object * obj, void * display) {
	ui_object * list = obj->target;
	if(((ui_toggle *)obj)->state & UI_TOGGLE_ON) {
		ui_set_toggle_state(obj, UI_TOGGLE_OFF);
		((ui_list_scroll *)list)->mode = UI_LIST_MODE_LIST;
		ui_reinitialize_object(list);
	} else {
		ui_set_toggle_state(obj, UI_TOGGLE_ON);
		((ui_list_scroll *)list)->mode = UI_LIST_MODE_GRID;
		ui_reinitialize_object(list);
		
	}
}

static void ui_scroll_next_click(ui_object * obj, void * display) {
	ui_object * list = obj->target;
	uint16 curTotal = 0;
	uint16 page_size ;
	ui_object * iterator = ((ui_object *)list)->child;
	uint16 startIdx = ((ui_list_scroll *)list)->cur_index;
	page_size = ((ui_list_scroll *)list)->page_length;
	//calculate total list item
	while(iterator != NULL) {
		iterator = iterator->sibling;
		curTotal ++;
	}
	if((((ui_list_scroll *)list)->cur_index + page_size) < curTotal && (curTotal > page_size))  {
		((ui_list_scroll *)list)->cur_index += page_size;
		//((ui_list_scroll *)list)->scroll_state = UI_SCROLL_STATE_DOWN;
#if HWCL_ENABLED && LCD_FMC_ENABLED
		ui_set_animation(list, UI_ANIM_SLIDE_LEFT);
#endif
		ui_reinitialize_object(list);
	}
}

static void ui_scroll_prev_click(ui_object * obj, void * display) {
	ui_object * list = obj->target;
	uint16 curTotal = 0;
	ui_object * iterator = ((ui_object *)list)->child;
	uint16 startIdx = ((ui_list_scroll *)list)->cur_index;
	uint16 page_size = ((ui_list_scroll *)list)->page_length;
	//calculate total list item
	while(iterator != NULL) {
		iterator = iterator->sibling;
		curTotal ++;
	}
	if(((ui_list_scroll *)list)->cur_index >= page_size) {
		((ui_list_scroll *)list)->cur_index -= page_size;
		//((ui_list_scroll *)list)->scroll_state = UI_SCROLL_STATE_UP;
#if HWCL_ENABLED && LCD_FMC_ENABLED
		ui_set_animation(list, UI_ANIM_SLIDE_RIGHT);
#endif
		ui_reinitialize_object(list);
	}
}

ui_object * ui_list_create(uint16 x, uint16 y, uint16 w, uint16 h) {
	//ui_list_scroll * obj = (ui_list_scroll *)ui_create_object(sizeof(ui_list_scroll), UI_TYPE_GRIDITEM | UI_ALIGN_GRID | UI_TYPE_CONTAINER) ;
	ui_rect wrect = {0, 0, 24, 24};
	ui_resource r1, r2;
	ui_list_view * obj = (ui_list_view *)ui_create_object(sizeof(ui_list_view), UI_ALIGN_FULL) ;
	((ui_object *)obj)->rect.x = x;
	((ui_object *)obj)->rect.y = y;
	((ui_object *)obj)->rect.w = w;
	((ui_object *)obj)->rect.h = h;
	((ui_object *)obj)->render = ui_list_view_render;
	((ui_object *)obj)->forecolor = UI_COLOR_BLUE;
	((ui_object *)obj)->backcolor = UI_COLOR_RGB(23,24,36);
	((ui_list_scroll *)obj)->scroll_state = 0;
	((ui_list_scroll *)obj)->cur_index = 0;
	((ui_list_scroll *)obj)->mode = UI_LIST_MODE_LIST;
	((ui_object *)obj)->handler = NULL;
	obj->list_area = (ui_list_scroll *)ui_list2_create(x, y, w, h);
	if(obj->list_area != NULL) {
		ui_set_align((ui_object *)obj->list_area, UI_ALIGN_FLOAT);
		ui_add_object((ui_object *)obj, (ui_object *)obj->list_area);
	}
	r1.buffer = (uint8 *)image_png_tile24;
	r1.size = sizeof(image_png_tile24);
	r2.buffer = (uint8 *)image_png_list24;
	r2.size = sizeof(image_png_list24);
	obj->view_btn = ui_tool_button_create(&wrect, &r1, &r2, ui_scroll_view_click);
	if(obj->view_btn != NULL) {
		ui_set_align(obj->view_btn, UI_ALIGN_FLOAT);
		ui_set_target(obj->view_btn, obj->list_area);
		ui_add_object((ui_object *)obj, obj->view_btn);
	}
	r1.buffer = (uint8 *)image_png_nav_next24;
	r1.size = sizeof(image_png_nav_next24);
	r2.buffer = (uint8 *)image_png_nav_nextd24;
	r2.size = sizeof(image_png_nav_nextd24);
	obj->next_btn = ui_tool_button_create(&wrect, &r1, &r2, ui_scroll_next_click);
	if(obj->next_btn != NULL) {
		ui_set_align(obj->next_btn, UI_ALIGN_FLOAT);
		ui_set_target(obj->next_btn, obj->list_area);
		ui_add_object((ui_object *)obj, obj->next_btn);
	}
	r1.buffer = (uint8 *)image_png_nav_prev24;
	r1.size = sizeof(image_png_nav_prev24);
	r2.buffer = (uint8 *)image_png_nav_prevd24;
	r2.size = sizeof(image_png_nav_prevd24);
	obj->prev_btn = ui_tool_button_create(&wrect, &r1, &r2, ui_scroll_prev_click);
	if(obj->prev_btn != NULL) {
		ui_set_align(obj->prev_btn, UI_ALIGN_FLOAT);
		ui_set_target(obj->prev_btn, obj->list_area);
		ui_add_object((ui_object *)obj, obj->prev_btn);
	}
	//if(click != NULL) ((ui_object *)obj)->handler = click;
	return (ui_object *)obj;
}