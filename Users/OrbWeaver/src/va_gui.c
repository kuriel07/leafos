#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\MMApis.h"	
#include "..\inc\VMStackApis.h"	
#include "..\crypto\inc\cr_apis.h"
#include "..\toolkit\inc\tk_apis.h"
#include "..\gui\inc\ui_core.h"
#include "..\core\inc\os_core.h"
#include "..\interfaces\inc\if_apis.h"
#include "..\inc\vm_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//extern vm_object * g_pVaRetval;
//extern sys_context g_sVaSysc;
extern tk_context_p g_pfrmctx;

//uint8 gb_va_state = VA_STATE_IDLE;
//static ui_object * gva_panels[16];
//static ui_object * g_current_screen = NULL;

typedef struct va_gui_context {
	uint8 state;
	ui_object * panels[16];
	ui_object * cur_screen;
} va_gui_context;

static void va_panel_click(ui_object *pnl, void * params) {
	VM_SET_ARG(pnl->target);
	vm_get_gui(va_gui_context *)->state = VA_STATE_RUN;
}

static void va_button_click(ui_object * btn, void * params) {
	VM_SET_ARG(btn->target);
	ui_button * clicked = (ui_button *)btn;
	vm_get_gui(va_gui_context *)->state = VA_STATE_RUN;
	va_return_word(VM_ARG, clicked->id);
}

static void va_button_ok_click(ui_object * btn, void * params) {
	VM_SET_ARG(btn->target);
	ui_button * clicked = (ui_button *)btn;
	vm_get_gui(va_gui_context *)->state = VA_STATE_RUN;
	va_return_word(VM_ARG, STK_RES_SUCCESS);
}
static void va_button_cancel_click(ui_object * btn, void * params) {
	VM_SET_ARG(btn->target);
	ui_button * clicked = (ui_button *)btn;
	vm_get_gui(va_gui_context *)->state = VA_STATE_RUN;
	va_return_word(VM_ARG, STK_RES_TERMINATED);
}

static void va_listitem_click(ui_object * item, void * params) {
	VM_SET_ARG(item->target);
	ui_item * clicked = (ui_item *)item;
	vm_get_gui(va_gui_context *)->state = VA_STATE_RUN;
	va_return_word(VM_ARG, clicked->id);
}

static void va_get_input_click(ui_object * btn, void * params) {
	ui_button * clicked = (ui_button *)btn;
	ui_textbox * textbox = btn->target;
	VM_SET_ARG(((ui_object *)textbox)->target);
	vm_get_gui(va_gui_context *)->state = VA_STATE_RUN;
	vm_set_retval(VM_NULL_OBJECT);					//default return null object
	if(clicked->id == STK_RES_SUCCESS) {
		vm_set_retval(vm_create_object(strlen((const char *)textbox->content), textbox->content));
	}
}

static void va_select_item_click(ui_object * itm, void * params) {
	VM_SET_ARG(itm->target);
	ui_item * selected = (ui_item *)itm;
	vm_get_gui(va_gui_context *)->state = VA_STATE_RUN;
	va_return_word(VM_ARG, selected->id);
}

ui_object * va_ui_current_screen(VM_DEF_ARG) {
	va_gui_context * gctx = NULL;
	if(VM_ARG == NULL) return NULL;
	if(VM_ARG->gui_ctx == NULL) return NULL;
	gctx = VM_ARG->gui_ctx;
	return gctx->cur_screen;
}

void va_select_item(VM_DEF_ARG) _REENTRANT_ {
	//param1 = title, param2 = array of stringuint8 tlen;
	OS_DEBUG_ENTRY(va_select_item);
	uint8 i,j;
	uint8 tag;
	uint16 size, tsize;
	uint8 tlen;
	ui_item * obj;
	vm_object * vtitle = vm_get_argument(VM_ARG, 0);
	vm_object * vlist = vm_get_argument(VM_ARG, 1);
	uint8 tbuf[VA_OBJECT_MAX_SIZE];
	ui_object * panel;
	gui_handle_p display;
	tk_context_p wctx = g_pfrmctx;
	if(ctx == NULL) goto exit_select_item;			//framework not initialized
	display = wctx->display;
	panel = ui_window_create(display, VM_ARG->name);
	//ui_set_object_name(panel, (const char *)VM_ARG->name);
	//tk_clear_body(ctx);
	//title label
	tlen = (vtitle->len > 255)? 255:vtitle->len;
	memcpy(tbuf, vtitle->bytes, tlen);	
	tbuf[tlen] = 0;
	ui_add_object(panel, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, tbuf));
	//list in array of string
	i = tk_pop(vlist->bytes, &tag, &size, tbuf);
	if(tag != ASN_TAG_SET) goto exit_select_item;
	for(i = 0, j = 0; i < size; j++) {
		i += tk_pop(tbuf + i, &tag, &tsize, tbuf);
		if((tag &0x7F) == ASN_TAG_OCTSTRING) {
			tbuf[tsize] = 0;
			ui_add_object(panel, (ui_object *)(obj = (ui_item *)ui_item_create(tbuf, NULL, 0, j, va_select_item_click)));
			ui_set_target((ui_object *)obj, VM_ARG);
		}
	}	
	//push new screen
	ui_push_screen(wctx->display, panel);
	vm_get_gui(va_gui_context *)->cur_screen = panel;
	//while((obj = ui_present(display)) == NULL);		//until an object selected
	va_ui_present(VM_ARG, g_pfrmctx, 0);
	vm_get_gui(va_gui_context *)->cur_screen = NULL;
	ui_pop_screen(wctx->display);							//pop screen
	exit_select_item:
	OS_DEBUG_EXIT();
	return;
}

void va_display_text(VM_DEF_ARG) _REENTRANT_ {
	//param1 = title, param2 = text
	OS_DEBUG_ENTRY(va_display_text);
	ui_item * obj;
	vm_object * vtitle = vm_get_argument(VM_ARG, 0);
	vm_object * vtext = vm_get_argument(VM_ARG, 1);
	uint8 mode = 0;
	ui_object * panel;
	gui_handle_p display;
	tk_context_p wctx = g_pfrmctx;
	uint8 tlen;
	uint8 numline = 1;
	uint16 i;
	uint8 tbuf[256];
	if(vm_get_argument_count(VM_ARG) > 2) {
		mode = va_o2f(vm_get_argument(VM_ARG, 2));
	}
	if(ctx == NULL) goto exit_display_text;			//framework not initialized
	display = wctx->display;
	panel = ui_window_create(display, VM_ARG->name);
	//ui_set_object_name(panel, (const char *)VM_ARG->name);
	//if((tlen = tk_pop_by_tag(tags, len, STK_TAG_ALPHA, tbuf)) != (uint8)-1) {
	tlen = (vtitle->len > 255)? 255:vtitle->len;
	memcpy(tbuf, vtitle->bytes, tlen);	
	tbuf[tlen] = 0;
	ui_add_object(panel, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, tbuf));
	
	tlen = (vtext->len > 255)? 255:vtext->len;
	memcpy(tbuf, vtext->bytes, tlen);	
	tbuf[tlen] = 0;
	switch(mode) {
		default:
		case 0x00:			//
		case 0x01:			//default text (CAT backward compatibility)
			for(i=0;i<vtext->len;i++) {
				if(tbuf[i] == '\n') numline++;
			}
			ui_add_object(panel, ui_label_create(UI_COLOR_WHITE, numline, UI_FONT_DEFAULT, tbuf));
			break;
		case 0x02:			//quick response code
			if(((gui_handle_p)wctx->display)->height < 96) break;
			ui_add_object(panel, ui_qrcode_create(UI_COLOR_BLACK, ((gui_handle_p)wctx->display)->height - 96, tbuf));
			break;
	}
	ui_add_object(panel, (ui_object *)(obj = (ui_item *)ui_buttonset_create(UI_BUTTON_SET_OK | UI_BUTTON_SET_CANCEL, 1, va_button_ok_click, va_button_cancel_click))) ;
	ui_set_target((ui_object *)obj, VM_ARG);
	//push new screen
	ui_push_screen(wctx->display, panel);
	vm_get_gui(va_gui_context *)->cur_screen = panel;
	//while((obj = ui_present(display)) == NULL);		//until an object selected
	va_ui_present(VM_ARG, g_pfrmctx, 0);
	vm_get_gui(va_gui_context *)->cur_screen = NULL;
	ui_pop_screen(wctx->display);							//pop screen
	exit_display_text:
	OS_DEBUG_EXIT();
	return;
}

void va_get_input(VM_DEF_ARG) _REENTRANT_ {
	//param1 = text, param2 = default contentuint8 tlen;
	OS_DEBUG_ENTRY(va_get_input);
	uint8 rlen;
	uint8 lbuf[256];
	uint8 tformat[128];
	uint8 tlen;
	uint8 tbuf[256];
	vm_object * vtitle = vm_get_argument(VM_ARG, 0);
	vm_object * vtext = vm_get_argument(VM_ARG, 1);
	uint8 mode = 0x01;
	uint8 maxlen = 128;
	ui_textbox * textbox;
	ui_object * obj;
	ui_object * panel;
	gui_handle_p display;
	tk_context_p wctx = g_pfrmctx;
	if(ctx == NULL) goto exit_get_input;			//framework not initialized
	if(vm_get_argument_count(VM_ARG) > 2) {
		mode = va_o2f(vm_get_argument(VM_ARG, 2));
	}
	display = wctx->display;
	panel = ui_window_create(display, VM_ARG->name);
	//ui_set_object_name(panel, (const char *)VM_ARG->name);
	//input label
	//if((tlen = tk_pop_by_tag(tags, len, STK_TAG_TEXT_STRING, tbuf)) != 255) {
	tlen = (vtitle->len > 255)? 255:vtitle->len;
	memcpy(tbuf, vtitle->bytes, tlen);	
	tbuf[tlen] = 0;
	ui_add_object(panel, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, tbuf));
	
	tlen = (vtext->len > 255)? 255:vtext->len;
	memcpy(tbuf, vtext->bytes, tlen);	
	tbuf[tlen] = 0;
	
	//standard input (text based)
	ui_add_object(panel, (ui_object *)(textbox = (ui_textbox *)ui_textbox_create((mode & 0x05), (uchar *)tbuf, maxlen, 4)));	
	ui_set_target((ui_object *)textbox, VM_ARG);
	ui_add_object(panel, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"Cancel", STK_RES_TERMINATED, va_get_input_click)));
	obj->type |= UI_ALIGN_GRID;
	ui_set_size(obj, 130, 48);
	obj->target = textbox;
	ui_add_object(panel, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"OK", STK_RES_SUCCESS, va_get_input_click)));
	obj->type |= UI_ALIGN_GRID;
	ui_set_size(obj, 130, 48);
	obj->target = textbox;
	//push new screen
	ui_push_screen(wctx->display, panel);
	vm_get_gui(va_gui_context *)->cur_screen = panel;
	//while((obj = ui_present(display)) == NULL);		//until an object selected
	va_ui_present(VM_ARG, g_pfrmctx, 0);
	vm_get_gui(va_gui_context *)->cur_screen = NULL;
	ui_pop_screen(wctx->display);							//pop screen
	exit_get_input:
	OS_DEBUG_EXIT();
	return;
}

//window management APIs, provide mechanism to create customized GUI from user application including windows management
void va_ui_init(VM_DEF_ARG) {
	uint8 i;
	va_gui_context * gui_ctx = os_alloc(sizeof(va_gui_context));
	if(gui_ctx != NULL) {
		VM_ARG->gui_ctx = gui_ctx;
		//clear all parameters in gui context
		vm_get_gui(va_gui_context *)->state = 0;
		for(i=0;i<16;i++) {
			vm_get_gui(va_gui_context *)->panels[i] = NULL;
		}
		vm_get_gui(va_gui_context *)->cur_screen = NULL;
	}
}

void va_ui_release_all(VM_DEF_ARG) {
	uint8 i;
	gui_handle_p display;
	if(g_pfrmctx == NULL) goto exit_ui_release_all;
	display = g_pfrmctx->display;
	if(VM_ARG == NULL) goto exit_ui_release_all;
	for(i=0;i<16;i++) {
		if( vm_get_gui(va_gui_context *)->panels[i] != NULL) {
			ui_remove_screen(display, vm_get_gui(va_gui_context *)->panels[i]);
			vm_get_gui(va_gui_context *)->panels[i] = NULL;
		}
	}
	vm_get_gui(va_gui_context *)->cur_screen = NULL;
	exit_ui_release_all:
	if(VM_ARG != NULL && VM_ARG->gui_ctx != NULL) {
		os_free(VM_ARG->gui_ctx);
		VM_ARG->gui_ctx = NULL;
	}
	return;
}

static uint8 va_ui_release(VM_DEF_ARG, ui_object * obj) {
	uint8 i;
	gui_handle_p display;
	if(g_pfrmctx == NULL) goto exit_ui_release;
	for(i=0;i<16;i++) {
		if(vm_get_gui(va_gui_context *)->panels[i] != obj) {
			if(vm_get_gui(va_gui_context *)->cur_screen == obj) 
				vm_get_gui(va_gui_context *)->cur_screen = NULL;
			//ui_delete_object(gva_panels[i]);
			ui_remove_screen(g_pfrmctx->display, obj);
			vm_get_gui(va_gui_context *)->panels[i] = NULL;
			return 0;
		}
	}
	exit_ui_release:
	return -1;
}

static uint8 va_ui_push_panel(VM_DEF_ARG, ui_object * obj) {
	uint8 i;
	for(i=0;i<16;i++) {
		if(vm_get_gui(va_gui_context *)->panels[i] == NULL) {
			vm_get_gui(va_gui_context *)->panels[i]  = obj;
			return 0;
		}
	}
	return -1;
}

static void va_ui_set_property(ui_object * obj, vm_object * vprop) {
	uint8 key[128];
	uint16 length;
	uint8 * val;
	void * enumerator;
	uint8 bbuf[VA_OBJECT_MAX_SIZE];
	if((vprop->mgc_refcount & 0xF0) != VM_OBJ_MAGIC) return;
	enumerator = va_arg_enumerate(vprop);
	while((val = va_arg_next(enumerator, key, &length, bbuf)) != NULL) {
		if(vm_imemcmp(key, "name", 4) == 0) {
			strncpy((char *)obj->name, (const char *)bbuf, UI_MAX_OBJECT_NAME - 1);
		} else if(vm_imemcmp(key, "x", 1) == 0) {
			obj->type &= ~UI_ALIGN_HORIZONTAL;	//remote horizontal alignment
			obj->rect.x = atoi((const char *)bbuf);
		} else if(vm_imemcmp(key, "y", 1) == 0) {
			obj->type &= ~UI_ALIGN_VERTICAL;	//remove vertical alignment
			obj->rect.y = atoi((const char *)bbuf);
		} else if(vm_imemcmp(key, "w", 1) == 0) {
			obj->type &= ~UI_ALIGN_FULL;		//remove auto-adjust size
			obj->rect.w = atoi((const char *)bbuf);
		} else if(vm_imemcmp(key, "h", 1) == 0) {
			obj->type &= ~UI_ALIGN_FULL;		//remove auto-adjust size
			obj->rect.h = atoi((const char *)bbuf);
		} else if(vm_imemcmp(key, "text", 1) == 0) {
			if(obj->type & UI_TYPE_TEXT) {
				strncpy(((ui_text *)obj)->text, (const char *)bbuf, UI_MAX_TEXT - 1);
			}
		}
	}
	va_arg_end(enumerator);
}

void va_ui_decode_property(VM_DEF_ARG, ui_object * parent, vm_object * props) {
	uint8 i,j;
	uint8 tag;
	uint16 size, tsize, ssize;
	uint8 tlen;
	uint8 tbuf[VA_OBJECT_MAX_SIZE];
	uint8 type[4];
	uint8 * image = NULL;
	uint16 img_size;
	uint16 id;
	uint32 align;
	uint32 mode;
	uint8 text[UI_MAX_TEXT + 1];
	uint8 name[UI_MAX_OBJECT_NAME + 1];
	uint16 x, y, w, h;
	ui_object * obj;
	//list in array of string
	i = tk_pop(props->bytes, &tag, &size, tbuf);
	if(tag != ASN_TAG_SET) goto exit_decode_property;
	for(i = 0; i < size;) {
		i += tk_pop(tbuf + i, &tag, &tsize, tbuf);
		if(tag == ASN_TAG_SEQ) {
			tbuf[tsize] = 0;
			image = NULL;
			img_size = 0;
			memset(type, 0, sizeof(type));
			memset(text, 0, sizeof(text));
			memset(name, 0, sizeof(name));
			id = 0;
			align = 0;
			mode = 0;
			x = 0;
			y = 0;
			w = 0;
			h = 0;
			obj = NULL;
			//start decoding property
			for(j = 0; j < tsize;) {
				j += tk_pop(tbuf + j, &tag, &ssize, tbuf);
				tbuf[ssize] = 0;
				if(tag == ASN_TAG_OBJDESC) {
					if(strstr((const char *)tbuf, "type") == (char *)tbuf) {
						memcpy((char *)type, tbuf + 5, 3);
						type[3] = 0;
					}
					else if(strstr((const char *)tbuf, "image") == (char *)tbuf) {
						image = os_alloc(ssize- 6);
						if(image != NULL) {
							memcpy((char *)image, tbuf + 6, ssize - 6);
							img_size = ssize - 6;
						}
					}
					else if(strstr((const char *)tbuf, "id") == (char *)tbuf) {
						id = atoi((const char *)tbuf + 3);
					}
					else if(strstr((const char *)tbuf, "align") == (char *)tbuf) {
						align = atoi((const char *)tbuf + 6);
					}
					else if(strstr((const char *)tbuf, "mode") == (char *)tbuf) {
						mode = atoi((const char *)tbuf + 5);
					}
					else if(strstr((const char *)tbuf, "text") == (char *)tbuf) {
						//mode = atoi((const char *)tbuf + 5);
						strncpy((char *)text, (const char *)tbuf + 5, UI_MAX_TEXT);
					}
					else if(strstr((const char *)tbuf, "name") == (char *)tbuf) {
						strncpy((char *)name, (const char *)tbuf + 5, UI_MAX_OBJECT_NAME);
					}
					else if(strstr((const char *)tbuf, "x") == (char *)tbuf) {
						x = atoi((const char *)tbuf + 2);
					}
					else if(strstr((const char *)tbuf, "y") == (char *)tbuf) {
						y = atoi((const char *)tbuf + 2);
					}
					else if(strstr((const char *)tbuf, "w") == (char *)tbuf) {
						w = atoi((const char *)tbuf + 2);
					}
					else if(strstr((const char *)tbuf, "h") == (char *)tbuf) {
						h = atoi((const char *)tbuf + 2);
					}
				}
			}
			if(strcmp((const char *)type, "lbl") == 0) {
				obj = ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, text);
			} else if(strcmp((const char *)type, "btn") == 0) {
				obj = ui_button_create(UI_COLOR_WHITE, text, id, va_button_click);
			} else if(strcmp((const char *)type, "txt") == 0) {
				obj = ui_textbox_create(mode, text, UI_MAX_TEXT, 4);
			} else if(strcmp((const char *)type, "lvi") == 0) {
				obj = ui_item_create(text, image, img_size, id, va_listitem_click);
			} else if(strcmp((const char *)type, "img") == 0) {
				obj = ui_picture_create(text, ((gui_handle_p)parent->display)->fb_pxfmt, img_size, image, va_panel_click);
			}
			if(image != NULL) { os_free(image); image = NULL; }
			if(obj != NULL) {
				obj->rect.w = w;
				obj->rect.h = h;
				obj->rect.x = x;
				obj->rect.y = y;
				ui_set_target(obj, VM_ARG);
				ui_set_align(obj, align);
				ui_add_object(parent, obj);
			}
		}
	}	
	
	exit_decode_property:
	return;
}

void va_ui_create_window(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_create_window);
	ui_object * obj;
	gui_handle_p display;
	if(g_pfrmctx == NULL) goto exit_create_window;
	display = g_pfrmctx->display;
	obj = ui_window_create(display, VM_ARG->name);
	if(obj == NULL) goto exit_create_window;
	ui_set_object_name(obj, (const char *)VM_ARG->name);
	ui_set_text((ui_text *)obj, (uint8 *)VM_ARG->name);
	if(va_ui_push_panel(VM_ARG, obj) == 0) {
		obj->handler = va_panel_click;							//default click event
		if(vm_get_argument_count(VM_ARG) >= 1) {
			va_ui_decode_property(VM_ARG, obj, vm_get_argument(VM_ARG, 0));
		}
		vm_set_retval(vm_create_object(sizeof(ui_object *), &obj));
		ui_set_target(obj, VM_ARG);
	} else {
		//unable to add panel
		os_free(obj);
	}
	exit_create_window:
	OS_DEBUG_EXIT();
}

void va_ui_destroy_window(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_destroy_window);
	ui_object * obj;
	gui_handle_p display;
	ui_object * temp;
	ui_rect i_area;
	ui_object * target;
	vm_object * handle = vm_get_argument(VM_ARG, 0);		//handle to current window
	//check for framework context
	if(g_pfrmctx == NULL) goto exit_destroy_window;
	display = g_pfrmctx->display;
	//check for target handle
	if(handle->len == 0) goto exit_destroy_window;
	memcpy(&target, handle->bytes, sizeof(ui_object *));
	if(target == NULL) goto exit_destroy_window;
	//remove from active/stacked screen if any (automatically release memory
	//ui_remove_screen(display, target);		
	//release window from memory (include window management)
	if(va_ui_release(VM_ARG, target) == 0) {
		//update mutator to null object
		vm_update_mutator(VM_ARG, target, VM_NULL_OBJECT);
	}
	exit_destroy_window:
	OS_DEBUG_EXIT();
	return;
}

void va_ui_create_label(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_create_label);
	ui_object * obj;
	gui_handle_p display;
	ui_object * parent;
	uint8 numline = 1;
	uint16 i;
	uint8 btext[VA_OBJECT_MAX_SIZE];
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	vm_object * text = vm_get_argument(VM_ARG, 1);
	int32 mode = va_o2f(vm_get_argument(VM_ARG, 2));
	if(g_pfrmctx == NULL) goto exit_create_label;
	display = g_pfrmctx->display;
	if(handle->len == 0) goto exit_create_label;
	memcpy(&parent, handle->bytes, sizeof(ui_object *));
	if(parent == NULL) goto exit_create_label;
	memcpy(btext, text->bytes, text->len);
	btext[text->len] = 0;
	switch(mode) {
		case 0x02:			//mode 2 QR code
			obj = ui_qrcode_create(UI_COLOR_WHITE, 96, btext);
			break;
		case 0x01:			//mode 1 multiline
		default:			//mode 0 default
			for(i=0;i<text->len;i++) {
				if(btext[i] == '\n') numline++;
			}
			obj = ui_dynamic_label_create(UI_COLOR_WHITE, numline, UI_FONT_DEFAULT, btext);
			break;
	}
	if(obj == NULL) goto exit_create_label;
	if((text->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
		va_ui_set_property(obj, text);
	}
	obj->handler = va_panel_click;
	ui_add_object(parent, obj);
	ui_set_target(obj, VM_ARG);
	vm_set_retval(vm_create_object(sizeof(ui_object *), &obj));
	exit_create_label:
	OS_DEBUG_EXIT();
	return;
}

void va_ui_create_button(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_create_button);
	ui_object * obj;
	gui_handle_p display;
	ui_object * parent;
	uint8 btext[VA_OBJECT_MAX_SIZE];
	vm_object * handle = vm_get_argument(VM_ARG, 0);		//parent
	vm_object * text = vm_get_argument(VM_ARG, 1);			//text
	uint8 id = va_o2f(vm_get_argument(VM_ARG, 2));			//id
	if(g_pfrmctx == NULL) goto exit_create_button;
	display = g_pfrmctx->display;
	if(handle->len == 0) goto exit_create_button;
	memcpy(&parent, handle->bytes, sizeof(ui_object *));
	if(parent == NULL) goto exit_create_button;
	memcpy(btext, text->bytes, text->len);
	btext[text->len] = 0;
	obj = ui_button_create(UI_COLOR_WHITE, btext, id, va_button_click);
	if(obj == NULL) goto exit_create_button;
	if((text->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
		va_ui_set_property(obj, text);
	}
	ui_add_object(parent, obj);
	ui_set_target(obj, VM_ARG);
	vm_set_retval(vm_create_object(sizeof(ui_object *), &obj));
	exit_create_button:
	OS_DEBUG_EXIT();
	return;
}

void va_ui_create_listitem(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_create_listitem);
	ui_object * obj;
	gui_handle_p display;
	ui_object * parent;
	uint8 btext[VA_OBJECT_MAX_SIZE];
	vm_object * handle = vm_get_argument(VM_ARG, 0);		//parent
	vm_object * text = vm_get_argument(VM_ARG, 1);			//text
	uint8 id = va_o2f(vm_get_argument(VM_ARG, 2));			//id
	if(g_pfrmctx == NULL) goto exit_create_listitem;
	display = g_pfrmctx->display;
	if(handle->len == 0) goto exit_create_listitem;
	memcpy(&parent, handle->bytes, sizeof(ui_object *));
	if(parent == NULL) goto exit_create_listitem;
	memcpy(btext, text->bytes, text->len);
	btext[text->len] = 0;
	obj = ui_item_create(btext, NULL, 0, id, va_listitem_click);
	if(obj == NULL) goto exit_create_listitem;
	if((text->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
		va_ui_set_property(obj, text);
	}
	ui_add_object(parent, obj);
	ui_set_target(obj, VM_ARG);
	vm_set_retval( vm_create_object(sizeof(ui_object *), &obj));
	exit_create_listitem:
	OS_DEBUG_EXIT();
	return;
}

void va_ui_create_textbox(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_create_textbox);
	ui_object * obj;
	gui_handle_p display;
	ui_object * parent;
	uint8 btext[VA_OBJECT_MAX_SIZE];
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	vm_object * text = vm_get_argument(VM_ARG, 1);
	uint8 mode = va_o2f(vm_get_argument(VM_ARG, 2));
	if(g_pfrmctx == NULL) goto exit_create_textbox;
	display = g_pfrmctx->display;
	if(handle->len == 0) goto exit_create_textbox;
	memcpy(&parent, handle->bytes, sizeof(ui_object *));
	if(parent == NULL) goto exit_create_textbox;
	memcpy(btext, text->bytes, text->len);
	btext[text->len] = 0;
	obj = ui_textbox_create(mode, btext, 128, 4);
	if(obj == NULL) goto exit_create_textbox;
	if((text->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
		va_ui_set_property(obj, text);
	}
	ui_add_object(parent, obj);
	ui_set_target(obj, VM_ARG);
	vm_set_retval(vm_create_object(sizeof(ui_object *), &obj));
	exit_create_textbox:
	OS_DEBUG_EXIT();
	return;
}

void va_ui_create_image(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_create_image);
	ui_object * obj;
	gui_handle_p display;
	ui_object * parent;
	ui_object * panel;
	uint8 btext[VA_OBJECT_MAX_SIZE];
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	vm_object * text = vm_get_argument(VM_ARG, 1);
	vm_object * img = vm_get_argument(VM_ARG, 2);
	if(g_pfrmctx == NULL) goto exit_create_image;
	display = g_pfrmctx->display;
	if(handle->len == 0) goto exit_create_image;
	memcpy(&parent, handle->bytes, sizeof(ui_object *));
	memcpy(btext, text->bytes, text->len);
	btext[text->len] = 0;
	obj = 	ui_picture_create(btext, ((gui_handle_p)display)->fb_pxfmt, img->len, img->bytes, va_panel_click);	//ui_textbox_create(mode, btext, 128, 4);
	if(obj == NULL) goto exit_create_image;
	if((text->mgc_refcount & 0xF0) == VM_OBJ_MAGIC) {
		va_ui_set_property(obj, text);
	}
	ui_add_object(parent, obj);
	ui_set_target(obj, VM_ARG);
	vm_set_retval(vm_create_object(sizeof(ui_object *), &obj));
	exit_create_image:
	OS_DEBUG_EXIT();
}
void va_ui_find_by_name(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_find_by_name);
	ui_object * obj;
	gui_handle_p display;
	uint8 buffer[UI_MAX_TEXT];
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	vm_object * vname = vm_get_argument(VM_ARG, 1);
	if(handle->len == 0) return;
	if(g_pfrmctx == NULL) goto exit_find_by_name;
	display = g_pfrmctx->display;
	//memcpy(&obj, handle->bytes, sizeof(ui_object *));
	strncpy((char *)buffer, (const char *)vname->bytes, UI_MAX_OBJECT_NAME -1);
	obj = ui_get_object_by_name(display, (const char *)buffer);
	if(obj != NULL) {
		vm_set_retval(vm_create_object(sizeof(ui_object *), &obj));
	}
	exit_find_by_name:
	OS_DEBUG_EXIT();
}

void va_ui_get_text(VM_DEF_ARG) {
	ui_text * obj;
	uint8 buffer[UI_MAX_TEXT];
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	if(handle->len == 0) return;
	memcpy(&obj, handle->bytes, sizeof(ui_text *));
	if(((ui_object *)obj)->type & UI_TYPE_TEXT) {
		strncpy((char *)buffer, obj->text, UI_MAX_TEXT);
		vm_set_retval(vm_create_object(strlen((const char *)buffer), buffer));
	}
}

void va_ui_set_text(VM_DEF_ARG) {
	ui_text * obj;
	uint8 buffer[UI_MAX_TEXT];
	uint8 vlen;
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	vm_object * vtext = vm_get_argument(VM_ARG, 1);
	if(handle->len == 0) return;
	memcpy(&obj, handle->bytes, sizeof(ui_text *));
	if(vlen > (UI_MAX_TEXT - 1)) vlen = (UI_MAX_TEXT - 1);
	memcpy((char *)buffer, vtext->bytes, vlen);
	buffer[vlen] = 0;
	ui_set_text(obj, buffer);
	ui_reinitialize_object((ui_object *)obj);
}

void va_ui_set_image(VM_DEF_ARG) {
	ui_object * obj;
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	vm_object * vimg = vm_get_argument(VM_ARG, 1);
	if(handle->len == 0) return;
	memcpy(&obj, handle->bytes, sizeof(ui_object *));
	ui_set_image(obj, vimg->bytes, vimg->len);
	ui_reinitialize_object((ui_object *)obj);
}

void va_ui_wait(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_wait);
	ui_object * obj;
	gui_handle_p display;
	ui_object * panel;
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	int32 ms = va_o2f(vm_get_argument(VM_ARG, 1));
	if(g_pfrmctx == NULL) goto exit_wait;
	display = g_pfrmctx->display;
	if(handle->len == 0) goto exit_wait;
	memcpy(&panel, handle->bytes, sizeof(ui_object *));
	if(panel == NULL) goto exit_wait;
	//push new screen
	ui_push_screen(display, panel);
	vm_get_gui(va_gui_context *)->cur_screen = panel;
	//while((obj = ui_present(display)) == NULL);		//until an object selected
	va_ui_present(VM_ARG, g_pfrmctx, ms);
	vm_get_gui(va_gui_context *)->cur_screen = NULL;
	ui_pop_screen_unsafe(display);							//pop screen but window is not released (unsafe)
	exit_wait:
	//g_pVaRetval = vm_create_object(sizeof(ui_object *), &obj);
	OS_DEBUG_EXIT();
	return;
}

void va_ui_present(VM_DEF_ARG, tk_context_p wctx, int32 ms) {
	int32 tick = ms / 20;
	vm_get_gui(va_gui_context *)->state = VA_STATE_IDLE;
	if(ms != 0) {
		//with timeout
		while(vm_get_gui(va_gui_context *)->state == VA_STATE_IDLE && vm_is_running(VM_ARG) && tick != 0) {
			if(vm_is_aborted(VM_ARG)) break;
			//ui_process_events(ctx->display);
			if_delay(20);
			tick--;
		}
	} else {
		//wait until user event
		while(vm_get_gui(va_gui_context *)->state == VA_STATE_IDLE && vm_is_running(VM_ARG)) {
			if(vm_is_aborted(VM_ARG)) break;
			//ui_process_events(ctx->display);
			if_delay(20);
		}
	}
}

void va_ui_push_window(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_push_window);
	ui_object * obj;
	gui_handle_p display;
	ui_object * panel;
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	if(g_pfrmctx == NULL) goto exit_push_window;
	display = g_pfrmctx->display;
	if(handle->len == 0) goto exit_push_window;
	memcpy(&panel, handle->bytes, sizeof(ui_object *));
	if(panel == NULL) goto exit_push_window;
	//push new screen
	ui_push_screen(display, panel);
	vm_get_gui(va_gui_context *)->cur_screen = panel;
	exit_push_window:
	//g_pVaRetval = vm_create_object(sizeof(ui_object *), &obj);
	OS_DEBUG_EXIT();
	return;
}

void va_ui_pop_window(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_pop_window);
	ui_object * obj;
	gui_handle_p display;
	ui_object * panel;
	vm_object * handle = vm_get_argument(VM_ARG, 0);
	if(g_pfrmctx == NULL) goto exit_pop_window;
	display = g_pfrmctx->display;
	if(handle->len == 0) goto exit_pop_window;
	memcpy(&panel, handle->bytes, sizeof(ui_object *));
	if(panel == NULL) goto exit_pop_window;
	if(display->body != panel) goto exit_pop_window;
	vm_get_gui(va_gui_context *)->cur_screen = NULL;
	ui_pop_screen_unsafe(display);							//pop screen but window is not released (unsafe)
	exit_pop_window:
	//g_pVaRetval = vm_create_object(sizeof(ui_object *), &obj);
	OS_DEBUG_EXIT();
	return;
}

void va_ui_alert(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_ui_alert);
	ui_object * obj;
	gui_handle_p display;
	ui_alert * alert;
	char result_buf[10];
	char text[256];
	char title[64];
	vm_object * obj_title = vm_get_argument(VM_ARG, 0);
	vm_object * obj_text = vm_get_argument(VM_ARG, 1);
	uint16 mode = va_o2f(vm_get_argument(VM_ARG, 2));
	uint8 icon = va_o2f(vm_get_argument(VM_ARG, 3));
	if(g_pfrmctx == NULL) goto exit_alert;
	display = g_pfrmctx->display;
	if(display == NULL) goto exit_alert;
	memset(text, 0, sizeof(text));
	memset(title, 0, sizeof(text));
	if((mode & (UI_ALERT_BUTTON_OK | UI_ALERT_BUTTON_CANCEL)) == 0) mode |= UI_ALERT_BUTTON_OK;
	if(obj_text->len != 0) memcpy(text, obj_text->bytes, ((obj_text->len > (sizeof(text) - 1))?sizeof(text) - 1:obj_text->len));
	if(obj_title->len != 0) memcpy(title, obj_title->bytes, ((obj_title->len > (sizeof(title) - 1))?sizeof(title) - 1:obj_title->len));
	alert = ui_alert_show(display, (uint8 *)title, (uint8 *)text, mode, icon);
	//while((obj = ui_present(display)) == NULL);		//until an object selected
	while((obj = ui_process_events(display)) == NULL) {
		if_delay(20);
	}
	ui_alert_close(display, alert);
	sprintf(result_buf, "%d", ui_alert_result(display));
	vm_set_retval(vm_create_object(strlen(result_buf), result_buf));
	exit_alert:
	OS_DEBUG_EXIT();
	return;
}

void va_delay(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_delay);
	uint16 tick = va_o2f(vm_get_argument(VM_ARG, 0));
	os_wait(tick);
	OS_DEBUG_EXIT();
}
