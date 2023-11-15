#ifdef WIN32
//#include "stdafx.h"
#endif
#include "..\..\defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "if_apis.h"
#include "..\inc\ui_core.h"
#include "..\inc\ui_toolbox.h"
#include "..\inc\ui_signal.h"
#include "..\inc\ui_textbox.h"
#include "..\inc\ui_panel.h"

//ui_config g_uconfig;
static ui_object * g_uobj_selected = NULL;

uint8 ui_add_header(gui_handle_p display, void * object) {
	uint32 psw;
	ui_object * iterator;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	iterator = display->header->child;
	if(iterator == NULL) display->header->child = object;
	else {
		while(iterator->sibling != NULL) {
			iterator = iterator->sibling;
		}
		iterator->sibling = object;
	}
	((ui_object *)object)->parent = display->header;
	((ui_object *)object)->display = display;
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return 0;
}

static void ui_clearheader(gui_handle_p display, uint32 color) {
	uchar i;
	uint32 psw;
	void (* release)(void *);
	ui_object * temp;
	ui_object * iterator = display->header;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	while(iterator != NULL) {
		temp = iterator;
		iterator = iterator->sibling;
		//free(temp);
		if(temp->release != NULL) {
			temp->release(temp);
		} 
		memset(temp, 0, sizeof(ui_object));
		os_free(temp);
	}
	display->header = NULL;
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return;
}


void ui_detach_object(ui_object * obj) {
	ui_object * temp;
	uint32 psw;
	void (* release)(void *);
	ui_object * iterator = obj;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	while(iterator != NULL) {
		if(iterator->magic != UI_OBJECT_MAGIC) {
			//invalid ui_object
			goto exit_detach_object;
		}
		temp = iterator;
		if(temp->child != NULL) ui_detach_object(temp->child);
		temp->child = NULL;			//zeroing memory
		iterator = iterator->sibling;
		//free(temp);
		temp->lcs = UI_LCS_DETACHED;
		//memset(temp, 0, sizeof(ui_object));
	}
	exit_detach_object:
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return;
}

void ui_delete_object(ui_object * obj) {
	ui_object * temp;
	uint32 psw;
	void (* release)(void *);
	ui_object * iterator = obj;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	while(iterator != NULL) {
		if(iterator->magic != UI_OBJECT_MAGIC) {
			//invalid ui_object
			goto exit_delete_object;
		}
		temp = iterator;
		if(temp->child != NULL) ui_delete_object(temp->child);
		temp->child = NULL;			//zeroing memory
		iterator = iterator->sibling;
		//free(temp);
		if(temp->img_release != NULL) {
			temp->image.size = 0;
			temp->img_release(temp->image.buffer);
		}
		if(temp->ac_foreground != NULL) {
			os_free(temp->ac_foreground);
		}
		if(temp->type & UI_TYPE_PANEL) {
			if(((ui_panel *)temp)->thumbnail != NULL) os_free(((ui_panel *)temp)->thumbnail);
			((ui_panel *)temp)->thumbnail = NULL;
		}
		//custom release for custome properties of object
		if(temp->release != NULL) {
			temp->release(temp);
		}
		memset(temp, 0, sizeof(ui_object));
		os_free(temp);
		//memset(temp, 0, sizeof(ui_object));
	}
	exit_delete_object:
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return;
}

static void ui_reinit_objects(ui_object * obj) {
	uint32 psw;
	ui_object * iterator = obj;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	while(iterator != NULL) {
		if(iterator->child != NULL) ui_reinit_objects(iterator->child);
		iterator->state |= UI_STATE_INIT;
		iterator = iterator->sibling;
	}
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

void ui_reinitialize_object(ui_object * obj) {
	ui_object * iterator;
	uint32 psw;
	if(obj == NULL) return;
	if(obj->magic != UI_OBJECT_MAGIC) return;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	ui_reinit_objects(obj->child);
	obj->state |= UI_STATE_INIT;
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

void ui_display_init(gui_handle_p handle) {
	handle->header = ui_toolbar_create(0, 0, 60, handle->height);
	handle->body = ui_create_panel(sizeof(ui_panel), 60, 24, handle->width - 60, handle->height - 24, (uint8 *)"home", 0, 0);
	handle->body->backcolor = UI_COLOR_RGB(23,24,36);
	ui_set_object_name(handle->body, "home");
}

void ui_card_init(gui_handle_p handle, void * cardctx) {
	//ui_object * signal;
	//ui_add_header(handle, (signal = ui_signal_create(UI_COLOR_BLUE, (uint8 *)"", 50, NULL)));
	//((ui_signal *)signal)->nctx = (net_context_p)netctx;
	//return NULL;
}

static void ui_release(void * ptr) {
	
	os_free(ptr);
}

void * ui_create_object(size_t objsize, uint32 mode) {
	ui_object * obj = (ui_object *)os_alloc(objsize);
	memset(obj, 0, objsize);
	obj->release = NULL;
	obj->magic = UI_OBJECT_MAGIC;
	obj->lcs = UI_LCS_AVAILABLE;
	obj->child = NULL;
	obj->sibling = NULL;
	obj->parent = NULL;
	obj->type = mode | UI_TYPE_VISIBLE;		//default always visible
	obj->state = UI_STATE_INIT;
	obj->image.buffer = NULL;
	obj->image.size = 0;
	obj->img_release = NULL;
	obj->display = NULL;
	
	//animation core
	obj->ac_foreground = NULL;
	obj->ac_background = NULL;
	obj->ac_state = UI_ANIM_NONE;
	obj->ac_duration = 0;
	return obj;
}

void * ui_create_text(size_t objsize, uint32 mode, char * text) {
	ui_text * obj = (ui_text *)ui_create_object(objsize, mode);
	if(obj == NULL) return NULL;
	((ui_object *)obj)->type |= UI_TYPE_TEXT;
	if(strlen((const char *)text) < UI_MAX_TEXT) {
		strncpy((char *)obj->text, (const char *)text, UI_MAX_TEXT);
	} else {
		memcpy((char *)obj->text, text, UI_MAX_TEXT - 1);
		obj->text[UI_MAX_TEXT - 1] = 0;
	}
	return obj;
}

void ui_set_text(ui_text * obj, uint8 * text) {
	if(obj == NULL) return;
	if(((ui_object *)obj)->magic != UI_OBJECT_MAGIC) return;
	if((((ui_object *)obj)->type & UI_TYPE_TEXT) == 0) return;
	if(strlen((const char *)text) < UI_MAX_TEXT) {
		strncpy((char *)obj->text, (const char *)text, UI_MAX_TEXT);
	} else {
		memcpy((char *)obj->text, text, UI_MAX_TEXT - 1);
		obj->text[UI_MAX_TEXT - 1] = 0;
	}
}

void ui_set_image(ui_object * obj, uint8 * image, uint16 img_size) {
	uint8 * ptr;
	uint8 * iptr;
	if(obj == NULL) return;
	if(obj->magic != UI_OBJECT_MAGIC) return;
	ptr = obj->image.buffer;
	iptr = (uint8*)os_alloc(img_size);
	if(iptr == NULL) return;
	memcpy(iptr, image, img_size);
	obj->image.buffer = iptr;
	obj->image.size = img_size;
	if(obj->img_release != NULL) obj->img_release(ptr);
	obj->img_release = os_free;
}

void ui_clear_body(gui_handle_p display, ui_object * screen) {
	uint32 psw;
	if(display->body == NULL) return;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	if(screen == NULL) {
		screen = ui_get_screen_by_name(display, "home");
	}
	if(screen != NULL) {
		ui_delete_object(screen->child);
		screen->child = NULL;
		screen->state |= UI_STATE_INIT;
	}
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

void ui_clear_child(gui_handle_p display, ui_object * parent) {
	uint32 psw;
	if(display->body == NULL) return;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	if(parent != NULL) {
		ui_detach_object(parent->child);
		parent->child = NULL;
		parent->state |= UI_STATE_INIT;
	}
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

uint8 ui_add_body(gui_handle_p display, void * object) {
	ui_object * iterator = display->body->child;
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	iterator = display->body->child;
	if(iterator == NULL) display->body->child = object;
	else {
		while(iterator->sibling != NULL) {
			iterator = iterator->sibling;
		}
		iterator->sibling = object;
	}
	((ui_object *)object)->parent = display->body;
	((ui_object *)object)->display = display;
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return 0;
}

uint8 ui_add_object(ui_object * parent, ui_object * obj) {
	ui_object * iterator;
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	iterator = parent->child;
	if(iterator == NULL) parent->child = obj;
	else {
		while(iterator->sibling != NULL) {
			iterator = iterator->sibling;
		}
		iterator->sibling = obj;
	}
	((ui_object *)obj)->parent = parent;
	((ui_object *)obj)->display = parent->display;
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return 0;
}

void ui_set_toggle_state(ui_object * object, uint8 state) {
	ui_object * iterator;
	uint32 psw;
	if(object == NULL) return;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	((ui_toggle *)object)->state = state;
	ui_reinitialize_object(object);
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
#if HWCL_ENABLED && LCD_FMC_ENABLED
	ui_set_animation(object, UI_ANIM_ALPHA_BLENDING);
#endif
}

void ui_release_detached_objects(ui_object * obj) {
	ui_object * temp;
	ui_object * prev = NULL;
	ui_object * iterator = obj;
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
		if(iterator->child != NULL) ui_release_detached_objects(iterator->child);
		iterator = iterator->sibling;
	}
}

void ui_render_objects(gui_handle_p display, ui_object * iterator) {
	//calculate position and size based on parent size and padding
	uint16 x = 0;// = ((ui_rect *)iterator->parent)->x + ((ui_view *)iterator->parent)->pleft;
	uint16 y = 0; //= ((ui_rect *)iterator->parent)->y + ((ui_view *)iterator->parent)->ptop;
	uint16 dw = display->width; //= ((ui_rect *)iterator->parent)->w - ((ui_view *)iterator->parent)->pright;
	uint16 dh = display->height; //= ((ui_rect *)iterator->parent)->h - ((ui_view *)iterator->parent)->pbottom;
	uint16 xsta, ysta;
	ui_rect rect;
	ui_rect prerender_rect = { 0, 0, iterator->rect.w, iterator->rect.h };
	if(iterator == NULL) return;
	if(iterator->magic != UI_OBJECT_MAGIC) {
		//invalid ui_object detected;
		return ;
	}
	if(iterator->parent != NULL) {
		x = ((ui_rect *)iterator->parent)->x;
		y = ((ui_rect *)iterator->parent)->y;
		dw = ((ui_rect *)iterator->parent)->w;
		dh = ((ui_rect *)iterator->parent)->h;
	}
	//xsta = ((ui_rect *)iterator)->x ;
	//ysta = ((ui_rect *)iterator)->y ;
	xsta = x, ysta = y;
	while(iterator != NULL) {
#if false
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
#endif
		//sort layout calculate relative position first
		switch(iterator->type & UI_ALIGN_MASK) {
			case UI_ALIGN_NONE:			//relative
				((ui_rect *)iterator)->x = iterator->rect.x + xsta;
				((ui_rect *)iterator)->y = iterator->rect.y + ysta;
				((ui_rect *)iterator)->w = iterator->rect.w;
				((ui_rect *)iterator)->h = iterator->rect.h;
				break;
			case UI_ALIGN_FLOAT:		//absolute
				((ui_rect *)iterator)->x = iterator->rect.x;
				((ui_rect *)iterator)->y = iterator->rect.y;
				((ui_rect *)iterator)->w = iterator->rect.w;
				((ui_rect *)iterator)->h = iterator->rect.h;
				break;
			case UI_ALIGN_FULL:			//stretch
				((ui_rect *)iterator)->x = x;
				((ui_rect *)iterator)->y = y;
				((ui_rect *)iterator)->w = dw;
				((ui_rect *)iterator)->h = (dh - y) + ysta;
				break;
			case UI_ALIGN_VERTICAL:		//row
				//if(display->width > (xsta + dw)) ((ui_rect *)iterator)->w = display->width - xsta;
				//else ((ui_rect *)iterator)->w = dw;
				((ui_rect *)iterator)->w = dw;
				((ui_rect *)iterator)->x = xsta;
				((ui_rect *)iterator)->y = y;
				((ui_rect *)iterator)->h = iterator->rect.h;
				break;
			case UI_ALIGN_HORIZONTAL: 	//column
				//if(display->height > (ysta + dh)) ((ui_rect *)iterator)->h = display->height - ysta;
				//else ((ui_rect *)iterator)->h = dh;
				((ui_rect *)iterator)->h = dh;
				((ui_rect *)iterator)->y = ysta;
				((ui_rect *)iterator)->x = x;
				((ui_rect *)iterator)->w = iterator->rect.w;
				break;
			case UI_ALIGN_GRID:			//cell
				((ui_rect *)iterator)->w = iterator->rect.w;
				((ui_rect *)iterator)->h = iterator->rect.h;
				if(xsta == 0) {
					if(((ui_rect *)iterator)->w > dw)  ((ui_rect *)iterator)->w = dw;			//resize width if object is bigger than container
				}
				if((iterator->type & UI_TYPE_BUTTON) == UI_TYPE_BUTTON) {
					iterator->type |= UI_TYPE_BUTTON;
				}
				if((x + ((ui_rect *)iterator)->w) > (xsta + dw)) { x = xsta; y += ((ui_rect *)iterator)->h; }		//fixed (2016.05.29)
				//if((y + ((ui_rect *)iterator)->h) > display->height) y = 0;
				((ui_rect *)iterator)->y = y;
				((ui_rect *)iterator)->x = x;
				break;
		}
		//calculate absolute position
		//((ui_rect *)iterator)->x += xsta;
		//((ui_rect *)iterator)->y += ysta;
		//((ui_object *)iterator)->state |= UI_STATE_INIT | UI_STATE_SELECTED;
		if(iterator->type & UI_TYPE_VISIBLE) {
#if HWCL_ENABLED && LCD_FMC_ENABLED
			if(((ui_object *)iterator)->ac_duration != 0) {
				ui_fb_animate(display, iterator);
			} else {
#endif
				
#if SHARD_ENABLE_PRERENDERED
			
				//ui_rect_copy(((ui_object *)iterator)->prerendered, &prerender_rect, 
				if(((ui_object *)iterator)->prerendered != NULL) ui_fb_copy(display, ((ui_object *)iterator)->prerendered, (ui_rect *)iterator);		
#else
				((ui_object *)iterator)->render(iterator, display);
#endif
				//render childs if current object is not container
				if(iterator->child != NULL && (iterator->type & UI_TYPE_CONTAINER) == 0) ui_render_objects(display, iterator->child);
#if HWCL_ENABLED && LCD_FMC_ENABLED
			}
#endif
		}
		//sort layout calculate relative position first
		switch(iterator->type & UI_ALIGN_MASK) {
			case UI_ALIGN_FULL:
				x = xsta;
				y = ysta;
				break;
			case UI_ALIGN_VERTICAL:
				y += ((ui_rect *)iterator)->h;
				//x = xsta;
				break;
			case UI_ALIGN_HORIZONTAL: 
				//y = ysta;
				x += ((ui_rect *)iterator)->w;
				break;
			case UI_ALIGN_GRID:
				x += ((ui_rect *)iterator)->w;
				break;
		}
		//clear global state
		((ui_object *)iterator)->state &= ~(UI_STATE_INIT | UI_STATE_KEYUP | UI_STATE_DESELECTED | UI_STATE_SELECTED);
		if(((ui_object *)iterator)->state & UI_STATE_KEYDOWN) {
			((ui_object *)iterator)->state &= ~UI_STATE_KEYDOWN;			//clear keydown state
			((ui_object *)iterator)->state |= UI_STATE_KEYUP;				//set keyup state
#if SHARD_RTOS_ENABLED
			//if(((ui_object *)iterator)->handler != NULL) ((ui_object *)iterator)->handler(iterator, display);
#endif
		}
		//((ui_object *)iterator)->state &= ~;
		iterator = iterator->sibling;
	}	
}

void ui_render(gui_handle_p display, ui_object * parent) {
	ui_release_detached_objects(parent);		//release detached objects first
	ui_render_objects(display, parent);			//start rendering
}

static ui_object * ui_get_object_by_xy_s(ui_object * obj, uint16 x, uint16 y) {
	uint16 x1, x2, y1, y2;
	ui_object * tobj = NULL;
	if((obj->type & UI_TYPE_VISIBLE) == 0) return NULL;			//only if visible
	while(obj  != NULL) {
		x1 = ((ui_rect *)obj)->x;
		y1 = ((ui_rect *)obj)->y;
		x2 = (x1 + ((ui_rect *)obj)->w);
		y2 = (y1 + ((ui_rect *)obj)->h);
		if(x >= x1 && x <= x2)
			if(y >= y1 && y <= y2) {
				if(obj->child != NULL && (obj->type & UI_TYPE_CONTAINER) == 0) tobj = ui_get_object_by_xy_s(obj->child, x, y);
				if(tobj != NULL) return tobj;
				return obj;
			}
		obj = obj->sibling;
	}
	return NULL;
}

static ui_object * ui_get_object_by_name_s(ui_object * obj, uint8 * name) {
	uint16 x1, x2, y1, y2;
	ui_object * tobj = NULL;
	while(obj  != NULL) {
		x1 = ((ui_rect *)obj)->x;
		y1 = ((ui_rect *)obj)->y;
		x2 = (x1 + ((ui_rect *)obj)->w);
		y2 = (y1 + ((ui_rect *)obj)->h);
		if(strncmp((char *)obj->name, (const char *)name, UI_MAX_OBJECT_NAME -1) == 0) return obj;
		if(obj->child != NULL && (obj->type & UI_TYPE_CONTAINER) == 0) tobj = ui_get_object_by_name_s(obj->child, name);
		if(tobj != NULL) return tobj;
		obj = obj->sibling;
	}
	return NULL;
}

void ui_set_object_name(ui_object * obj, const char * name) {
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	strncpy( (char *)obj->name, (const char *)name, UI_MAX_OBJECT_NAME -1);
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

ui_object * ui_get_object_by_name(gui_handle_p display, const char * name) {
	//static ui_object * prevSelected = NULL;
	ui_object * obj = NULL;
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	obj = ui_get_object_by_name_s(display->header , (uint8 *)name);
		if(obj != NULL) goto exit_get_object;
	obj = ui_get_object_by_name_s(display->body, (uint8 *)name);
		if(obj != NULL) goto exit_get_object;
	obj = ui_get_object_by_name_s(display->meta, (uint8 *)name);
	exit_get_object:
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return obj;
}

ui_object * ui_get_object_by_xy(gui_handle_p display, uint16 x, uint16 y) {
	//static ui_object * prevSelected = NULL;
	ui_object * obj = NULL;
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	obj = ui_get_object_by_xy_s(display->body, x, y);
		if(obj != NULL) goto exit_get_object;
	obj = ui_get_object_by_xy_s(display->header , x, y);
		if(obj != NULL) goto exit_get_object;
	obj = ui_get_object_by_xy_s(display->meta , x, y);
	exit_get_object:
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return obj;
}

void ui_set_selected_object(gui_handle_p display, ui_object * obj) {
	uint32 psw;
	if(obj == NULL) return;
	//check for any touch state
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	if(g_uobj_selected != NULL) {
		g_uobj_selected->state &= (~UI_STATE_SELECTED);
		//trigger deselection
		g_uobj_selected->state |= UI_STATE_DESELECTED;
		if(g_uobj_selected->handler != NULL) g_uobj_selected->handler(obj, display);
		//g_uobj_selected->state &= (~UI_STATE_DESELECTED);
	}
	//select new object
	g_uobj_selected = obj;
	obj->state |= UI_STATE_SELECTED;
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

ui_object * ui_process_events(gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_process_events);
	uint16 x, y;
	ui_object * obj = NULL;
	//uint32 psw;
	//check for any touch state
#if SHARD_RTOS_ENABLED
	//psw = os_enter_critical();
#endif
	if(display->touch_state != NULL && ((gui_handle_p)display)->touch_state(display) & UI_KEY_DOWN) {
		if(display->touch_read != NULL && ((gui_handle_p)display)->touch_read(display, &x, &y) == 0) {
			obj = ui_get_object_by_xy(display, x, y);
			if(obj == NULL) goto exit_process;
			if((display->status & UI_STATUS_ACTIVE) == 0) goto exit_process;
			//set selection+click state
			obj->state |= (UI_STATE_KEYDOWN);
#if SHARD_RTOS_ENABLED
			//wait for keydown render
			while(obj->state & UI_STATE_KEYDOWN);// ui_present(display);		//wait for render
			while(obj->state & UI_STATE_KEYUP);// ui_present(display);		//wait for render
#endif
			if(obj->type & UI_TYPE_SELECTABLE) {
				ui_set_selected_object(display, obj);
			}
			if(obj->handler != NULL) obj->handler(obj, display);
			if(obj->type & UI_TYPE_CONTAINER) {
				obj = ((ui_container *)obj)->selected;
			}
			//obj->state &= (~UI_STATE_CLICKED);
		}
	}
	exit_process:
#if SHARD_RTOS_ENABLED
	//os_exit_critical(psw);
#endif
	OS_DEBUG_EXIT();
	return obj;
}

void * ui_present(gui_handle_p display) {
	BYTE i;
	uint16 x = 0, y = 24;
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	ui_render(display, display->body);
	ui_render(display, display->header);
	ui_render(display, display->meta);
	//while(1);			//should wait for object selection
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return NULL;
}

static void ui_load_thumbnail(gui_handle_p display, ui_panel * panel) {
	uint16 w = ((ui_rect *)panel)->w;
	uint16 h = ((ui_rect *)panel)->h;
	ui_size sz;
	ui_image * thumbnail;
	if(w > h) {
		w = 32;
		h = (((ui_rect *)panel)->h * w) / ((ui_rect *)panel)->w;
	} else {
		h = 32;
		w = (((ui_rect *)panel)->w * h) / ((ui_rect *)panel)->h;
	}
	sz.w = w;
	sz.h = h;
	if((((ui_object *)panel)->type & UI_TYPE_PANEL) == 0) return;
	thumbnail = panel->thumbnail;
	panel->thumbnail = NULL;
	if(thumbnail != NULL) os_free(thumbnail);		//delete previouse thumbnail
	panel->thumbnail = ui_fb_resize_image(display, (ui_rect *)panel, &sz); 
}

ui_object * ui_push_screen(gui_handle_p display, ui_object * panel) {
	ui_object * cur_screen = NULL;
	uint32 psw;
	ui_rect rect;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	if(display->stack_index < UI_MAX_STACK) {
		cur_screen = display->body;
		memcpy(&rect, &display->view_rect, sizeof(ui_rect));
		display->stack[display->stack_index++] = display->body;
		ui_load_thumbnail(display, (ui_panel *)display->body);
		if(panel != NULL) display->body = panel;
		else display->body = ui_create_panel(sizeof(ui_panel), rect.x, rect.y, rect.w, rect.h, (uint8 *)"", 0, 0);
		ui_reinitialize_object(display->body);
	}
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return display->body;
}

uint8 ui_check_overlapped(ui_rect * a, ui_rect * b) {
	if((b->x >= a->x && b->x < (a->x + a->w)) || (a->x <= (b->x + b->w) && a->x > b->x)) {
		if((b->y >= a->y && b->y < (a->y + a->h)) || (a->y <= (b->y + b->h) && a->y > b->y)) return 1; 
	}
	return 0;
}

uint8 ui_check_intersect(ui_rect * a, uint16 x, uint16 y) {
	if(x >= a->x && x <= (a->x + a->w)) {
		if(y >= a->y && y <= (a->y + a->h)) return 1;
	}
	return 0;
}

void ui_invalidate_area(ui_object * root, ui_rect * rect) {
	ui_object * iterator;
	uint32 psw;
	if(root == NULL) return;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	iterator = root;
	while(iterator != NULL) {
		if(ui_check_overlapped((ui_rect *)iterator, rect)) ui_reinitialize_object(iterator);
		if(iterator->child != NULL) ui_invalidate_area(iterator->child, rect);
		iterator = iterator->sibling;
	}
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	
}

void ui_clear_selected(gui_handle_p display) {
	g_uobj_selected = NULL;						//set no object selected
}

ui_object * ui_pop_screen_unsafe(gui_handle_p display) {
	ui_object * temp;
	ui_rect i_area;
	uint32 psw;
	if(display->stack_index == 0) return NULL;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	temp = display->stack[--display->stack_index];
	memcpy(&i_area, display->body, sizeof(ui_rect));
	display->body = temp;
	g_uobj_selected = NULL;						//set no object selected
	if(display->stack_index == 0) ui_invalidate_area(display->header, &i_area);		//reinitialize to be re-rendered
	ui_invalidate_area(display->body, &i_area);		//reinitialize to be re-rendered
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return temp;
}

ui_object * ui_pop_screen(gui_handle_p display) {
	ui_object * temp;
	ui_rect i_area;
	uint32 psw;
	if(display->stack_index == 0) return NULL;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	temp = display->stack[--display->stack_index];
	memcpy(&i_area, display->body, sizeof(ui_rect));
	ui_delete_object(display->body);
	display->body = temp;
	g_uobj_selected = NULL;						//set no object selected
	if(display->stack_index == 0) ui_invalidate_area(display->header, &i_area);		//reinitialize to be re-rendered
	ui_invalidate_area(display->body, &i_area);		//reinitialize to be re-rendered
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
	return temp;
}

ui_object * ui_get_screen_by_name(gui_handle_p display, const char * name) {
	uint8 i;
	ui_object * screen;
	if(strncmp((const char *)display->body->name, name, UI_MAX_OBJECT_NAME-1) == 0) {
		return display->body;
	}
	for(i=0;i<display->stack_index;i++) {
		screen = display->stack[i];
		if(strncmp((const char *)screen->name, name, UI_MAX_OBJECT_NAME-1) == 0) {
			return screen;
		}
	}
	return NULL;
}

uint8 ui_is_screen_exist(gui_handle_p display, ui_object * screen) {
	uint8 i;
	if(screen == NULL) return FALSE;
	if(display->body == screen) return TRUE;
	for(i=0;i<display->stack_index;i++) {
		if(display->stack[i] == screen) return TRUE;
	}
	return FALSE;
}

void ui_remove_screen(gui_handle_p display, ui_object * screen) {
	if(ui_remove_screen_unsafe(display, screen) != NULL) {
		ui_delete_object(screen);
	}
	if(display->body != NULL) {
		ui_reinitialize_object(display->body);
	}
}

ui_object * ui_remove_screen_unsafe(gui_handle_p display, ui_object * screen) {
	uint8 i;
	uint8 index = 0;
	if(screen == NULL) return NULL;
	//remove screen from display list
	for(i=0;i<display->stack_index;i++) {
		if(display->stack[i] == screen) {
			display->stack[i] = NULL;
		}
	}
	//compacting display stack
	for(i=0;i<display->stack_index;i++) {
		if(display->stack[index] == NULL) {
			display->stack[index] = display->stack[i];
		}
		if(display->stack[index] != NULL) {
			index++;
		}
	}
	display->stack_index = index;
	//pop screen from display if current
	if(display->body == screen) {
		ui_pop_screen_unsafe(display);
		ui_reinitialize_object(display->body);
	}
	return screen;
}


ui_object * ui_wait_user_input(gui_handle_p display) {
	OS_DEBUG_ENTRY(ui_wait_user_input);
	ui_object * triggered_obj  = NULL;
	triggered_obj = NULL;
	display->status |= UI_STATUS_WAIT;
	while(display->status & UI_STATUS_WAIT) {
		triggered_obj = ui_process_events(display);
		os_wait(67);			//14~15 fps
		//if(triggered_obj != NULL) {
		//	if(triggered_obj->type & (UI_TYPE_BUTTON |UI_TYPE_SELECTABLE)) break;
		//}
	}
	OS_DEBUG_EXIT();
	return triggered_obj;
}

void ui_clear_dispstack(gui_handle_p display) {
	ui_object * temp;
	uint32 psw;
	if(display->stack_index == 0) return;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	while(display->stack_index > 0) {
		temp = display->stack[--display->stack_index];
		ui_delete_object(display->body);
		display->body = temp;
	}
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

static void ui_set_bound(ui_object * obj, int16 w, int16 h) {
	ui_object * iterator;
	uint32 psw;
	if(obj == NULL) return;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	iterator = obj;
	while(iterator != NULL) {
		switch(obj->type & UI_ALIGN_MASK) {
			case UI_ALIGN_FLOAT:
				//if(((ui_rect *)obj)->w > w) ((ui_rect *)obj)->w = w;
				//if(((ui_rect *)obj)->h > h) ((ui_rect *)obj)->h = h;
				break;
			case UI_ALIGN_VERTICAL:
				((ui_rect *)obj)->w = w;
				break;
			case UI_ALIGN_HORIZONTAL:
				((ui_rect *)obj)->h = h;
				break;
			case UI_ALIGN_GRID:
				
				break;
			default: break;
		}
		if(iterator->child != NULL) ui_set_bound(iterator->child, w - ((ui_rect *)iterator)->x, h - ((ui_rect *)iterator)->y);
		iterator = iterator->sibling;
	}
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}


void ui_set_align(ui_object * obj, uint32 align) {
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	if(obj->magic == UI_OBJECT_MAGIC) {
		obj->type &= ~UI_ALIGN_MASK;
		obj->type |= align;
	}
	//ui_set_bound(obj->child, w, h);
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

void ui_set_target(ui_object * obj, void * target) {
	obj->target = target;
}

void ui_set_size(ui_object * obj, uint16 w, uint16 h) {
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	//((ui_rect *)obj)->w = w;
	//((ui_rect *)obj)->h = h;
	obj->rect.w = w;
	obj->rect.h = h;
	//ui_set_bound(obj->child, w, h);
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

void ui_set_position(ui_object * obj, uint16 x, uint16 y) {
	uint32 psw;
#if SHARD_RTOS_ENABLED
	psw = os_enter_critical();
#endif
	//if((obj->type & UI_ALIGN_MASK) == UI_ALIGN_NONE) {
	obj->rect.x = x;
	obj->rect.y = y;
	//}
#if SHARD_RTOS_ENABLED
	os_exit_critical(psw);
#endif
}

void ui_switch_orientation(gui_handle_p display, uint8 orientation) {
	//ui_rect * rect;
	uint8 i;
	ui_object * screen;
	uint16 long_edge, short_edge;
	short_edge = display->height;
	long_edge = display->width;
	uint16 header_w;
	uint16 meta_h;
	display->orientation = orientation & 0x03;
	if(display->width >= 240) {
		meta_h = 24;
	} else {
		meta_h = 16;
	}
	if(long_edge < short_edge) {
		short_edge = display->width;
		long_edge = display->height;
	}
	if(orientation & 0x01) {
		display->width = short_edge;
		display->height = long_edge;
	} else {
		display->height = short_edge;
		display->width = long_edge;
	}
	header_w = short_edge / 4;		//calculate header width (shorter edge / 4, ex: edge =240, width=60)
	if(orientation & 0x01) {		//vertical orientation
		//reposition header
		ui_set_position(((gui_handle_p)display)->header, 0, ((gui_handle_p)display)->height - header_w);
		ui_set_size(((gui_handle_p)display)->header, ((gui_handle_p)display)->width, header_w);
		if(((gui_handle_p)display)->meta != NULL) {
			ui_set_position(((gui_handle_p)display)->meta, 0, 0);
			ui_set_size(((gui_handle_p)display)->meta, ((gui_handle_p)display)->width, meta_h);
		}
		//set viewing rect
		((gui_handle_p)display)->view_rect.x = 0;
		((gui_handle_p)display)->view_rect.y = meta_h;
		((gui_handle_p)display)->view_rect.w = ((gui_handle_p)display)->width;
		((gui_handle_p)display)->view_rect.h = (((gui_handle_p)display)->height - meta_h) - header_w;
		//reposition body
		ui_set_position(((gui_handle_p)display)->body, 0, meta_h);
		ui_set_size(((gui_handle_p)display)->body, ((gui_handle_p)display)->width, (((gui_handle_p)display)->height - meta_h) - header_w);
		//re-initialize all display stack
		for(i=0;i<((gui_handle_p)display)->stack_index;i++) {
			ui_set_position(((gui_handle_p)display)->stack[i], 0, meta_h);
			ui_set_size(((gui_handle_p)display)->stack[i], ((gui_handle_p)display)->width, (((gui_handle_p)display)->height - meta_h) - header_w);
			ui_reinitialize_object(((gui_handle_p)display)->stack[i]);
		}
		
	} else {							//horizontal orientation
		//reposition header
		ui_set_position(((gui_handle_p)display)->header, 0, 0);
		ui_set_size(((gui_handle_p)display)->header, header_w, ((gui_handle_p)display)->height);
		if(((gui_handle_p)display)->meta != NULL) {
			ui_set_position(((gui_handle_p)display)->meta, header_w, 0);
			ui_set_size(((gui_handle_p)display)->meta, ((gui_handle_p)display)->width - header_w, meta_h);
		}
		//set viewing rect
		((gui_handle_p)display)->view_rect.x = header_w;
		((gui_handle_p)display)->view_rect.y = meta_h;
		((gui_handle_p)display)->view_rect.w = ((gui_handle_p)display)->width - header_w;
		((gui_handle_p)display)->view_rect.h = ((gui_handle_p)display)->height - meta_h;
		//reposition body
		ui_set_position(((gui_handle_p)display)->body, header_w, meta_h);
		ui_set_size(((gui_handle_p)display)->body, ((gui_handle_p)display)->width - header_w, ((gui_handle_p)display)->height - meta_h);
		//re-initialize all display stack
		for(i=0;i<((gui_handle_p)display)->stack_index;i++) {
			ui_set_position(((gui_handle_p)display)->stack[i], header_w, meta_h);
			ui_set_size(((gui_handle_p)display)->stack[i], ((gui_handle_p)display)->width - header_w, ((gui_handle_p)display)->height - meta_h);
			ui_reinitialize_object(((gui_handle_p)display)->stack[i]);
		}
	}
	display->switch_orientation(display, orientation);
}

//animation core
uint8 ui_get_pixel_size(uint16 pxfmt) {
	switch(pxfmt) {
		case UI_RGB565	: return 2;
		case UI_RGB888: return 3;
		case UI_RGB332: return 1;
		case UI_ARGB8888: return 4;
		default: break;
	}
	return 0;		//unsupported pixel
}

ui_image * ui_image_alloc(ui_size * sz, uint16 pxfmt) {
	uint8 pxsz = ui_get_pixel_size(pxfmt);
	uint32 img_sz = (sz->w * sz->h * pxsz);
	ui_image * img = os_alloc(img_sz + sizeof(ui_image) + 16);
	img->base.size.w = sz->w;
	img->base.size.h = sz->h;
	img->base.buffer = os_align_address(img->data);
	img->base.pxfmt = pxfmt;
	img->base.length = img_sz;
	return img;
}

void ui_rect_copy(ui_buffer * src, ui_rect * src_rect, ui_buffer * dst, ui_rect * dst_rect, uint16 pxfmt) {
	uint8 pxsz = ui_get_pixel_size(pxfmt);
	//uint32 src_sz = dst_rect->w * dst_rect->h * pxsz;
	uint8 * dstptr;
	uint8 * srcptr;
	uint16 x = 0, y = 0;
	uint16 dst_w = (dst->size.w > dst_rect->w)?dst_rect->w:dst->size.w;
	uint16 dst_h =  (dst->size.h > dst_rect->h)?dst_rect->h:dst->size.h;
	uint16 src_w = (src->size.w > src_rect->w)?src_rect->w:src->size.w;
	uint16 src_h =  (src->size.h > src_rect->h)?src_rect->h:src->size.h;
	uint16 w = (dst_w > src_w)?src_w:dst_w;
	uint16 h = (dst_h > src_h)?src_h:dst_h;
	//if(rect->x >= sz->w) return;
	//if(rect->y >= sz->h) return;
	//if(xend >= sz->w) xend = sz->w;
	//if(yend >= sz->h) yend = sz->h;
	if(w == 0) return;
	if(h == 0) return;
	for(y = 0; y < h; y++) {
		//calculate memory offset
		dstptr = dst->buffer + ((((dst_rect->y + y) * dst->size.w) + dst_rect->x) * pxsz);
		srcptr = src->buffer + ((((src_rect->y + y) * src->size.w) + src_rect->x) * pxsz);
		dma_memcpy(dstptr, srcptr, w * pxsz); 
		dma_wait();
		//dma_memcpy(dst + (((y - rect->y) * rect->w) * pxsz), src + ((y + rect->x) * pxsz), (xend - src_rect->x) * pxsz); 
		//memcpy(dst + (((y - rect->y) * rect->w) * pxsz), src + ((y + rect->x) * pxsz), (xend - src_rect->x) * pxsz); 
	}
}

void ui_fb_copy(gui_handle_p handle, ui_buffer * dst, ui_rect * rect) {
	ui_rect drect;
	ui_buffer src;
	if(handle == NULL) return;
	if(handle->fb_ptr == NULL) return;
	if(dst == NULL) return;
	src.buffer = handle->fb_ptr;
	src.size.w = handle->width;
	src.size.h = handle->height;
	drect.x = 0;
	drect.y = 0;
	drect.w = handle->width;
	drect.h = handle->height;
	ui_rect_copy(&src, rect, dst, &drect, handle->fb_pxfmt);
}

ui_image * ui_fb_crop_image(gui_handle_p handle, ui_rect * rect) {
	ui_size temp_sz;
	ui_image * image;
	gui_handle temp_display;
	if(handle == NULL) return NULL;
	if(handle->fb_ptr == NULL) return NULL;
	if(rect == NULL) return NULL;
	temp_sz.w = rect->w;
	temp_sz.h = rect->h;
	image = ui_image_alloc(&temp_sz, handle->fb_pxfmt);
	if(image == NULL) return NULL;
	ui_fb_copy(handle, (ui_buffer *)image, rect);
	return image;
}

void ui_fb_copy_resize_image(gui_handle_p handle, ui_buffer * dst, ui_rect * rect) {
	uint8 pxsz = ui_get_pixel_size(handle->fb_pxfmt);
	float w_r, h_r;		//width ratio, height ratio
	float x, y;
	uint32 i, j;
	uint16 w,h;
	uint8 * p11;
	uint8 * p12;
	uint8 * p21;
	uint8 * p22;
	uint16 p1, p2, pt, pb, p;
	uint8 * b;
	if(handle == NULL) return;
	if(handle->fb_ptr == NULL) return;
	if(rect == NULL) return;
	w_r = rect->w / dst->size.w;
	h_r = rect->h / dst->size.h;
	switch(pxsz) {
		case 1:
			for(y=0,j=0; y<rect->h && j<dst->size.h; y += h_r,j++) {
				for(x=0,i=0; x<rect->w && i<dst->size.w; x += w_r,i++) {
					*(uint8 *)((uint8 *)dst->buffer + (((j * dst->size.w) + i) * pxsz)) = 
					*(uint8 *)((uint8 *)handle->fb_ptr + ((((rect->y + (uint32)y) * handle->width) + (rect->x + (uint32)x)) * pxsz)); 
				}
			}
			break;
		case 2:
			//RGB565
			for(y=0,j=0; y<rect->h && j<dst->size.h; y += h_r,j++) {
				for(x=0,i=0; x<rect->w && i<dst->size.w; x += w_r,i++) {
					b = ((uint8 *)dst->buffer) + (((j * dst->size.w) + i) * pxsz);
					p11 = ((uint8 *)handle->fb_ptr) + ((((rect->y + (uint32)y) * handle->width) + (rect->x + (uint32)x)) * pxsz);
					p12 = ((uint8 *)handle->fb_ptr) + ((((rect->y + (uint32)y) * handle->width) + (rect->x + (uint32)x + (int16)w_r)) * pxsz);
					p21 = ((uint8 *)handle->fb_ptr) + ((((rect->y + (uint32)y + (int16)h_r) * handle->width) + (rect->x + (uint32)x)) * pxsz);
					p22 = ((uint8 *)handle->fb_ptr) + ((((rect->y + (uint32)y + (int16)h_r) * handle->width) + (rect->x + (uint32)x + (int16)w_r)) * pxsz);
					p1 = *((uint16 *)p11); 
					p2 = *((uint16 *)p12); 
					pt = ((p1 >> 1) & 0x7DEF) + ((p2 >> 1) & 0x7DEF);
					p1 = *((uint16 *)p21); 
					p2 = *((uint16 *)p22); 
					pb = ((p1 >> 1) & 0x7DEF) + ((p2 >> 1) & 0x7DEF);
					p = ((pt >> 1) & 0x7DEF) + ((pb >> 1) & 0x7DEF);
					b[0] = ((uint8 *)&p)[0];
					b[1] = ((uint8 *)&p)[1];
				}
			}
			break;
		case 3:
			for(y=0,j=0; y<rect->h; y += h_r,j++) {
				for(x=0,i=0; x<rect->w; x += w_r,i++) {
					//no interpolation
					b = (uint8 *)((uint8 *)dst->buffer + (((j * dst->size.w) + i) * pxsz));
					p11 = (uint8 *)((uint8 *)handle->fb_ptr + ((((rect->y + (uint32)y) * handle->width) + (rect->x + (uint32)x)) * pxsz)); 
					b[0] = p11[0];
					b[1] = p11[1];
					b[2] = p11[2];
				}
			}
			break;
	}
	return;
}

ui_image *  ui_fb_resize_image(gui_handle_p handle, ui_rect * rect, ui_size * size) {
	ui_image * image = NULL;
	if(handle == NULL) return NULL;
	if(handle->fb_ptr == NULL) return NULL;
	if(rect == NULL) return NULL;
	image = ui_image_alloc(size, handle->fb_pxfmt);
	if(image == NULL) return NULL;
	ui_fb_copy_resize_image(handle, (ui_buffer *)image, rect) ;
	return image;
}

void ui_fill_image(gui_handle_p handle, ui_rect * rect, ui_image * image) {
	uint16 i, j;
	uint16 * dwptr;
	uint32 * dxptr;
	uint8 pxsz;
	handle->set_area(handle, rect->x, rect->y, rect->w, rect->h);
	handle->begin_burst(handle);					//prepare for write
	pxsz = ui_get_pixel_size(image->base.pxfmt);
	switch(pxsz) {
		case 2:
			for(i=0;i<handle->height && i < rect->h;i++) {
				for(j=0;j<handle->width && j<rect->w;j++) {
					dwptr = (uint16 *)((uint8 *)image->data + (((i * rect->w) + j) * pxsz));
					handle->put_pixel(handle, dwptr[0]);
				}
			}
			break;
		case 3:
			for(i=0;i<handle->height && i < rect->h;i++) {
				for(j=0;j<handle->width && j<rect->w;j++) {
					dxptr = (uint32 *)((uint8 *)image->data + (((i * rect->w) + j) * pxsz));
					handle->put_pixel(handle, dxptr[0] & 0x00FFFFFF);
				}
			}
			break;
		case 4:
			for(i=0;i<handle->height && i < rect->h;i++) {
				for(j=0;j<handle->width && j<rect->w;j++) {
					dxptr = (uint32 *)((uint8 *)image->data + (((i * rect->w) + j) * pxsz));
					handle->put_pixel(handle, dxptr[0]);
				}
			}
			break;
		
	}
}

void ui_fb_setup_animation(ui_object * obj) {
	if(obj == NULL) return;
	if(obj->ac_foreground != NULL) os_free(obj->ac_foreground);
	obj->ac_foreground = NULL;
	if(obj->ac_background != NULL) os_free(obj->ac_background);
	obj->ac_background = NULL;
}

void ui_set_animation(ui_object * obj, uint16 mode) {
	if(obj == NULL) return;
	switch(mode) {
		case UI_ANIM_ALPHA_BLENDING:
			obj->ac_state = mode;
			obj->ac_duration = 7;
			ui_fb_setup_animation(obj);
			break;
		case UI_ANIM_SLIDE_LEFT:
		case UI_ANIM_SLIDE_RIGHT:
			obj->ac_state = mode;
			obj->ac_duration = 5;
			ui_fb_setup_animation(obj);
			break;
	}
}

uint8 ui_fb_animate(gui_handle_p handle, ui_object * obj) {
	HAL_StatusTypeDef res;
	DMA2D_HandleTypeDef * hwcl;
	ui_image * imgbuf = NULL;
	ui_size temp_sz;
	ui_image * image;
	ui_rect temp_rect;
	ui_buffer sbuf;
	ui_rect dst_rect, src_rect;
	gui_handle temp_display;
	uint32 alpha = 0x20;
	uint32 slide_w;
	uint32 output_pxfmt;
	uint32 input_pxfmt;
	if(obj == NULL) return 0;
	obj->ac_duration--;
	//check for ac_duration or ac_support, start render immediately if not animation supported
	if(obj->ac_duration == 0 || (obj->ac_state & handle->ac_support) == 0) {
		obj->ac_state = UI_ANIM_NONE;
		obj->ac_duration = 0;
		if(obj->ac_foreground != NULL) {
			os_free(obj->ac_foreground);
			obj->ac_foreground = NULL;
		}
		if(obj->ac_background != NULL) {
			os_free(obj->ac_background);
			obj->ac_background = NULL;
		}
		ui_reinitialize_object(obj);
		obj->render(obj, handle);
		return 0;
	}
	temp_sz.w = obj->rect.w;
	temp_sz.h = obj->rect.h;
	((ui_rect *)obj)->w = obj->rect.w;
	((ui_rect *)obj)->h = obj->rect.h;
	//start animation process in memory
	hwcl = ((DMA2D_HandleTypeDef *)handle);
	switch(obj->ac_state) {
		case UI_ANIM_ALPHA_BLENDING:
			//start pre-render background only on ui_fb_animate, remember single buffer PNG decoder
			if(obj->ac_background == NULL) {
				obj->ac_background = ui_fb_crop_image(handle, (ui_rect *)obj);
				if(obj->ac_background == NULL) {
					obj->ac_duration = 0;
					return 0;
				}
			}
			//start pre-render foreground only on ui_fb_animate, remember single buffer PNG decoder
			if(obj->ac_foreground == NULL) {
				image = ui_image_alloc(&temp_sz, handle->fb_pxfmt);
				if(image == NULL) { 
					os_free(obj->ac_background);
					obj->ac_duration = 0;
					return 0;
				}
				obj->ac_foreground = image;
				//save object rect
				memcpy(&temp_rect, ((ui_rect *)obj), sizeof(ui_rect));
				((ui_rect *)obj)->x = 0;
				((ui_rect *)obj)->y = 0;
				//re-route display handler to current object image
				memcpy(&temp_display, handle, sizeof(gui_handle));		//copy to temporary
				temp_display.height = temp_sz.h;
				temp_display.width = temp_sz.w;
				temp_display.fb_ptr = image->data;
				temp_display.fb_size = image->base.length;
				//ask for object to start rendering
				obj->render(obj, &temp_display);
				//restore object rect
				memcpy(((ui_rect *)obj), &temp_rect, sizeof(ui_rect));
			}
			//calculate alpha value for blending
			if(obj->ac_duration < 7) alpha = 255 - (obj->ac_duration * 0x20);
			if(obj->ac_duration == 1) alpha = 255;
			switch(((gui_handle_p)handle)->fb_pxfmt) {
				case UI_RGB565: output_pxfmt = DMA2D_OUTPUT_RGB565; input_pxfmt = DMA2D_INPUT_RGB565; break;
				case UI_RGB888: output_pxfmt = DMA2D_OUTPUT_RGB888; input_pxfmt = DMA2D_INPUT_RGB888; break;
				case UI_ARGB8888: output_pxfmt = DMA2D_OUTPUT_ARGB8888; input_pxfmt = DMA2D_INPUT_ARGB8888; break;
			}
			//obj->ac_state = mode;
			//obj->ac_duration = 5;
			//ui_fb_render_object(handle, obj);
			hwcl->Instance = DMA2D;
			hwcl->Init.Mode = DMA2D_M2M_BLEND;
			hwcl->Init.ColorMode = output_pxfmt;				//LCD output
			hwcl->Init.OutputOffset = 0;
			hwcl->XferCpltCallback  = NULL;		//no callback
			hwcl->XferErrorCallback = NULL;		//no callback
			//foreground layer configuration
			hwcl->LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;		//top layer combine alpha with background
			hwcl->LayerCfg[1].InputAlpha = alpha;
			hwcl->LayerCfg[1].InputColorMode = input_pxfmt;
			hwcl->LayerCfg[1].InputOffset = 0;
			
			/* Background layer Configuration */
			hwcl->LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;		//background layer, no alpha transparency
			hwcl->LayerCfg[0].InputAlpha = 255; /* 127 : semi-transparent */
			hwcl->LayerCfg[0].InputColorMode = input_pxfmt;	//with transparency
			hwcl->LayerCfg[0].InputOffset = 0x0; /* No offset in input */
			if (HAL_DMA2D_Init(hwcl) != HAL_OK)
			{
				//Error_Handler();
				while(1);
			}
			/* Apply DMA2D Foreground configuration */
			HAL_DMA2D_ConfigLayer(hwcl, 1);
			/* Apply DMA2D Background configuration */
			HAL_DMA2D_ConfigLayer(hwcl, 0);
			temp_sz.w = obj->rect.w;
			temp_sz.h = obj->rect.h;
			imgbuf = ui_image_alloc(&temp_sz, handle->fb_pxfmt);
			if(imgbuf != NULL) {
				HAL_DMA2D_BlendingStart(hwcl,
                                          (uint32_t)obj->ac_foreground->data,
                                          (uint32_t)obj->ac_background->data,
                                          (uint32_t)imgbuf->data,
                                          obj->rect.w,
                                          obj->rect.h);
				//HAL_DMA2D
			}
			HAL_DMA2D_PollForTransfer(hwcl, 10);
			ui_fill_image(handle, (ui_rect *)obj, imgbuf);	
			os_free(imgbuf);
			//HAL_DMA2D_DeInit(hwcl);
			break;
		case UI_ANIM_SLIDE_LEFT:
			//start pre-render foreground only on ui_fb_animate, remember single buffer PNG decoder
			if(obj->ac_foreground == NULL) {
				image = ui_image_alloc(&temp_sz, handle->fb_pxfmt);
				if(image == NULL) { 
					os_free(obj->ac_background);
					obj->ac_duration = 0;
					return 0;
				}
				obj->ac_foreground = image;
				//save object rect
				memcpy(&temp_rect, ((ui_rect *)obj), sizeof(ui_rect));
				((ui_rect *)obj)->x = 0;
				((ui_rect *)obj)->y = 0;
				//re-route display handler to current object image
				memcpy(&temp_display, handle, sizeof(gui_handle));		//copy to temporary
				temp_display.height = temp_sz.h;
				temp_display.width = temp_sz.w;
				temp_display.fb_ptr = image->data;
				temp_display.fb_size = image->base.length;
				ui_reinitialize_object(obj);
				//ask for object to start rendering
				obj->render(obj, &temp_display);
				//restore object rect
				memcpy(((ui_rect *)obj), &temp_rect, sizeof(ui_rect));
			}
			alpha = obj->base.w / 5;
			if(obj->ac_duration > 5) obj->ac_duration = 5;
			if(obj->ac_duration < 5) slide_w = obj->ac_duration * alpha;
			//if(obj->ac_duration == 1) slide_w = 0;
			sbuf.buffer = handle->fb_ptr;
			sbuf.size.w = handle->width;
			sbuf.size.h = handle->height;
			sbuf.pxfmt = handle->fb_pxfmt;
			sbuf.length = handle->fb_size;
			//calculate destination and source rect for background
			memcpy(&dst_rect, obj, sizeof(ui_rect));
			memcpy(&src_rect, obj, sizeof(ui_rect));
			src_rect.x = src_rect.x + alpha;
			src_rect.w = slide_w;
			dst_rect.w = slide_w;
			//src_rect.x = src_rect.x + src_rect.w - alpha
			ui_rect_copy(&sbuf, &src_rect, &sbuf, &dst_rect, sbuf.pxfmt);
			//calculate destination and source rect for foreground
			memcpy(&dst_rect, obj, sizeof(ui_rect));
			memcpy(&src_rect, obj, sizeof(ui_rect));
			src_rect.x = 0;
			src_rect.y = 0;
			//src_rect.w += slide_w - alpha;
			src_rect.w += src_rect.w - slide_w;
			dst_rect.x = dst_rect.x + slide_w;
			dst_rect.w = dst_rect.w - slide_w;
			ui_rect_copy((ui_buffer *)obj->ac_foreground, &src_rect, &sbuf, &dst_rect, sbuf.pxfmt);
			break;
		case UI_ANIM_SLIDE_RIGHT:
			//start pre-render foreground only on ui_fb_animate, remember single buffer PNG decoder
			if(obj->ac_foreground == NULL) {
				image = ui_image_alloc(&temp_sz, handle->fb_pxfmt);
				if(image == NULL) { 
					os_free(obj->ac_background);
					obj->ac_duration = 0;
					return 0;
				}
				obj->ac_foreground = image;
				//save object rect
				memcpy(&temp_rect, ((ui_rect *)obj), sizeof(ui_rect));
				((ui_rect *)obj)->x = 0;
				((ui_rect *)obj)->y = 0;
				//re-route display handler to current object image
				memcpy(&temp_display, handle, sizeof(gui_handle));		//copy to temporary
				temp_display.height = temp_sz.h;
				temp_display.width = temp_sz.w;
				temp_display.fb_ptr = image->data;
				temp_display.fb_size = image->base.length;
				ui_reinitialize_object(obj);
				//ask for object to start rendering
				obj->render(obj, &temp_display);
				//restore object rect
				memcpy(((ui_rect *)obj), &temp_rect, sizeof(ui_rect));
			}
			alpha = obj->base.w / 5;
			if(obj->ac_duration > 5) obj->ac_duration = 5;
			if(obj->ac_duration < 5) slide_w = obj->ac_duration * alpha;
			//if(obj->ac_duration == 1) slide_w = 0;
			sbuf.buffer = handle->fb_ptr;
			sbuf.size.w = handle->width;
			sbuf.size.h = handle->height;
			sbuf.pxfmt = handle->fb_pxfmt;
			sbuf.length = handle->fb_size;
			//calculate destination and source rect for background
			memcpy(&dst_rect, obj, sizeof(ui_rect));
			memcpy(&src_rect, obj, sizeof(ui_rect));
			src_rect.x = src_rect.x + (src_rect.w - slide_w) - alpha;
			src_rect.w = slide_w + alpha;
			dst_rect.x = dst_rect.x + (dst_rect.w - slide_w);
			dst_rect.w = slide_w;
			//src_rect.x = src_rect.x + src_rect.w - alpha
			ui_rect_copy(&sbuf, &src_rect, &sbuf, &dst_rect, sbuf.pxfmt);
			//calculate destination and source rect for foreground
			memcpy(&dst_rect, obj, sizeof(ui_rect));
			memcpy(&src_rect, obj, sizeof(ui_rect));
			src_rect.x = 0;
			src_rect.y = 0;
			src_rect.x = src_rect.x + (src_rect.w - (src_rect.w - slide_w));
			src_rect.w = (src_rect.w - slide_w);
			dst_rect.x = dst_rect.x;
			dst_rect.w = dst_rect.w - slide_w;
			ui_rect_copy((ui_buffer *)obj->ac_foreground, &src_rect, &sbuf, &dst_rect, sbuf.pxfmt);
			break;
	}
}