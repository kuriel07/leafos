#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\gui\inc\ui_gauge.h"
#include "..\..\gui\inc\ui_resources.h"
#include "..\..\orbweaver\inc\vmstackapis.h"
#include "..\..\orbweaver\inc\vm_framework.h"

extern uint8 gb_tk_state;
extern uint8 gba_registered_menu[256];
extern uint8 gb_registered_size;

static void tk_button_ok_click(ui_object * btn, void * params) {
	//set terminal response (during display_text)
	((ui_button *)btn)->id = STK_RES_SUCCESS;
	gb_tk_state = TK_STATE_DISPLAY_TEXT;
}

static void tk_button_cancel_click(ui_object * btn, void * params) {
	//set terminal response (during display_text)
	((ui_button *)btn)->id = STK_RES_TERMINATED;
	gb_tk_state = TK_STATE_DISPLAY_TEXT;
}

static void tk_menu_item_click(ui_object * item, void * params) {
	gb_tk_state = TK_STATE_ENVELOPE_MENU;
}

static void tk_get_input_handler(ui_object * tbox, void * params) {
	//during get input
	gb_tk_state = TK_STATE_GET_INPUT;
}

static void tk_get_datetime_handler(ui_object * dbox, void * params) {
	//during get input
	gb_tk_state = TK_STATE_GET_DATETIME;
}

static void tk_select_item_click(ui_object * item, void * params) {
	//during select item view
	gb_tk_state = TK_STATE_SELECT_ITEM;		//back to list application
}

static void tk_card_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_card_click);
	os_task * task;
	ui_object * cf_btn;
	uint8 show_marker = FALSE;
	//check if card already connected
	ui_object * screen;
	tk_context_p ctx = (tk_context_p)obj->target;
	if(if_card_state(ctx->cctx) == 0) {
		gb_tk_state = TK_STATE_LIST_APP;
		show_marker = TRUE;
	} else {
		task = os_find_task_by_name((char *)ctx->config->devlet_name);
		if(task == NULL) {
			ctx->cos_status &= ~TK_COS_STAT_PICC_AVAIL;
			tk_kernel_exec(ctx);
			show_marker = TRUE;
		} else {
			if(task->context != NULL && vm_is_running(task->context)) {
				screen = va_ui_current_screen(task->context);
				if(ui_is_screen_exist(ctx->display, screen) &&((gui_handle_p)ctx->display)->body != screen) {
					ui_remove_screen_unsafe(ctx->display, screen);
					ui_push_screen(ctx->display, screen);
					show_marker = TRUE;
				} else {
					ctx->cos_status &= ~TK_COS_STAT_VM_STARTED;
					vm_abort(task->context);
					show_marker = FALSE;
					//while(vm_is_running(task->context)) os_wait(40);
				}
			} else {
				ctx->cos_status &= ~TK_COS_STAT_PICC_AVAIL;
				tk_kernel_exec(ctx);
				show_marker = TRUE;
			}
		}
	}
	cf_btn = ui_get_object_by_name(ctx->display, "myCard") ;
	if(cf_btn != NULL) {
		((ui_icon_tool *)cf_btn)->show_marker = show_marker;
	}
	OS_DEBUG_EXIT();
}

ui_object * tk_set_action(tk_context_p ctx, uint8 * text, uint8 * img, uint32 imgsize, void (* action)(ui_object * obj, void * params)) {
	OS_DEBUG_ENTRY(tk_set_action);
	ui_icon_tool * btn = (ui_icon_tool *)ui_get_object_by_name(ctx->display, "myAction");
	if(btn != NULL) {
		if(text == NULL) text = (uint8 *)"";
		btn->bitmap[0].bitmap = img;
		btn->bitmap[0].bmpsize = imgsize;
		btn->bitmap[1].bitmap = img;
		btn->bitmap[1].bmpsize = imgsize;
		ui_set_text((ui_text *)btn, text);
		((ui_object *)btn)->handler = action;
		ui_reinitialize_object((ui_object *)btn);
	}
	OS_DEBUG_EXIT();
	return (ui_object *)btn;
}

void tk_display_text(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags) {
	OS_DEBUG_ENTRY(tk_display_text);
	ui_object * obj;
	BYTE tlen;
	uint8 tbuf[256];
	uint16 wbtn;
	tk_resource_record_p res;
	tk_clear_body(ctx);
	//print title (if exist)
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_ALPHA, tbuf)) != (uint8)-1) {
		tbuf[tlen] = 0;
		ui_add_body(ctx->display, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, tbuf));
	}
	//display resource (if exist)
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_RESOURCE_ID, tbuf)) != (uint8)-1) {
		res = tk_get_resource(ctx, tbuf[0]);
		ui_add_body(ctx->display, ui_picture_create((uint8 *)"", ((gui_handle_p)ctx->display)->fb_pxfmt, res->res_size, res->res_ptr, NULL));
	}
	//print description (if exist)
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_TEXT_STRING, tbuf)) != (uint8)-1) {
		tbuf[tlen] = 0;
		switch(mode) {
			default:
			case 0x00:			//
			case 0x01:			//default text (CAT backward compatibility)
				ui_add_body(ctx->display, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, tbuf + 1));
				break;
			case 0x02:			//quick response code
				if(((gui_handle_p)ctx->display)->height < 96) break;
				ui_add_body(ctx->display, ui_qrcode_create(UI_COLOR_BLACK, ((gui_handle_p)ctx->display)->height - 96, tbuf + 1));
				break;
		}
	}
	//add action button (cancel/ok)
	wbtn = ((ui_rect *)((gui_handle_p)ctx->display)->body)->w / 2;
	
	//ui_add_body(ctx->display, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"Cancel", STK_RES_TERMINATED, tk_menu_button_click)));
	//obj->id = STK_RES_SUCCESS;
	//ui_set_align(obj, UI_ALIGN_GRID);
	//ui_set_size(obj, wbtn, 48);
	//ui_add_body(ctx->display, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"OK", STK_RES_SUCCESS, tk_menu_button_click)));
	//obj->id = STK_RES_TERMINATED;
	//ui_set_align(obj, UI_ALIGN_GRID);
	//ui_set_size(obj, wbtn, 48);
	ui_add_body((gui_handle_p)ctx->display, (obj = ui_buttonset_create(UI_BUTTON_SET_OK | UI_BUTTON_SET_CANCEL, 1, tk_button_ok_click, tk_button_cancel_click))) ;
	gb_tk_state = TK_STATE_DISPLAY_TEXT;
	OS_DEBUG_EXIT();
	return;
}

void tk_get_input(tk_context_p ctx, uint8 len, uint8 qualifier, BYTE * tags) {
	OS_DEBUG_ENTRY(tk_get_input);
	uint8 tlen;
	uint8 rlen;
	uint8 lbuf[256];
	uint8 tformat[128];
	uint8 tbuf[256];
	uint8 maxlen = 128;
	uint16 wbtn;
	ui_textbox * textbox;
	ui_object * obj;
	ui_object * target;
	tk_clear_body(ctx);
	//input label
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_TEXT_STRING, tbuf)) != 255) {
		tbuf[tlen] = 0;
		ui_add_body(ctx->display, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, tbuf + 1));
	}
	//input default text
	tbuf[0] = 0;
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_DEFAULT_TEXT, tbuf)) != 255) {
		tbuf[tlen] = 0;
	}
	//datetime format
	tformat[0] = 0;
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_DATETIMEFORMAT, tformat)) != 255) {
		tformat[tlen] = 0;
	}
	//input response length
	if((rlen = tk_pop_by_tag(tags, len, STK_TAG_RESPONSE_LENGTH, lbuf)) != 255) {
		maxlen = lbuf[1];
	}
	wbtn = ((ui_rect *)((gui_handle_p)ctx->display)->body)->w / 2;
	if((qualifier & 0x08) == 0) {
		//standard input (text based)
		ui_add_body(ctx->display, (textbox = (ui_textbox *)ui_textbox_create((qualifier & 0x05), (uchar *)tbuf, maxlen, 4)));	
		ui_add_body(ctx->display, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"Cancel", STK_RES_TERMINATED, tk_get_input_handler)));
		ui_set_align(obj, UI_ALIGN_GRID);
		ui_set_size(obj, wbtn, 48);
		obj->target = textbox;
		ui_add_body(ctx->display, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"OK", STK_RES_SUCCESS, tk_get_input_handler)));
		ui_set_align(obj, UI_ALIGN_GRID);
		ui_set_size(obj, wbtn, 48);
		obj->target = textbox;
	} else {
		//special input
		switch(qualifier & 0x07) {
			case 0x00:			//datetime input
				target = (ui_object *)ui_datetime_item_create((uint8 *)"userDateTime", (uint8 *)tformat, (uint8 *)tbuf);
				if(target != NULL) {
					ui_add_body(ctx->display, target);	
					ui_add_body(ctx->display, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"Cancel", STK_RES_TERMINATED, tk_get_datetime_handler)));
					ui_set_align(obj, UI_ALIGN_GRID);
					ui_set_size(obj, wbtn, 48);
					obj->target = target;
					ui_add_body(ctx->display, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"OK", STK_RES_SUCCESS, tk_get_datetime_handler)));
					ui_set_align(obj, UI_ALIGN_GRID);
					ui_set_size(obj, wbtn, 48);
					obj->target = target;
				}
				break;
			default: break;
		}
	}
	gb_tk_state = TK_STATE_GET_INPUT;
	OS_DEBUG_EXIT();
}

void tk_select_item(tk_context_p ctx, BYTE len, BYTE * tags) {
	OS_DEBUG_ENTRY(tk_select_item);
	uint8 tlen;
	uint8 i;
	uint8 tag;
	uint16 size;
	ui_item * obj;
	uint8 tbuf[70];
	tk_clear_body(ctx);
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_ALPHA, tbuf)) != 255) {
		tbuf[tlen] = 0;
		ui_add_body(ctx->display, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, tbuf));
	}
	for(i =0; i<len; ) {
		i += tk_pop(tags + i, &tag, &size, tbuf);
		if((tag &0x7F) == STK_TAG_ITEM) {
			tbuf[size] = 0;
			//need to fixed this (item number always added by one, so terminal need to decrease 1)
			ui_add_body(ctx->display, (obj = (ui_item *)ui_item_create(tbuf + 1, NULL, 0, tbuf[0] -1, tk_select_item_click)));
		}
	}	
	gb_tk_state = TK_STATE_SELECT_ITEM;
	OS_DEBUG_EXIT();
}

uint8 tk_local_info(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags) {
	OS_DEBUG_ENTRY(tk_local_info);
	//3GPP 11.14
	//PROVIDE LOCAL INFORMATION
	//'00' = Location Information (MCC, MNC, LAC and Cell Identity)
	//'01' = IMEI of the ME
	//'02' = Network Measurement results
	//'03' = Date, time and time zone	
	//'04' = Language setting
	//'05' = Timing Advance
	//'06' to 'FF' = Reserved
	datetime dtime;
	uint8 ret = 0;
	switch(mode) {
		case 1:			//ESID
			memcpy(tags, ctx->esid, SHARD_ESID_SIZE);
			ret = SHARD_ESID_SIZE;
			break;
		case 3:			//ISO8601 datetime
			if(if_time_is_running()) {	//RTC is not synchronized (running)
				if_time_get(&dtime);
				tk_iso8601_encode(&dtime, tags);
				ret = strlen((const char *)tags);
			}
			break;
		default:
			break;
	}
	OS_DEBUG_EXIT();
	return ret;
}

static void tk_infostat_render(ui_object * obj, gui_handle_p display) {
	OS_DEBUG_ENTRY(tk_infostat_render);
	uint16 xx, yy;
	uint16 x = ((ui_rect *)obj)->x;
	uint16 y = ((ui_rect *)obj)->y;
	uint16 w = ((ui_rect *)obj->parent)->w - 24;
	uint16 h = ((ui_rect *)obj)->h;
	uint8 i;
	uint16 j;
	uint16 xlen, x1, wtxt, nc;
	//uint8 cntr = 0;
	uint8 linebuf[27];
	uint16 wgauge = w / 4;
	tk_context_p ctx = (tk_context_p)obj->target;
	uint16 percent;
	ui_signal * signal = NULL;
	ui_object mold;
	//static uint8 cpu_util = -1;
	//recalculate width and height
	h = ((ui_rect *)obj)->h = ((ui_rect *)obj->parent)->h;
	if(obj->sibling != NULL) {
		//should check for number of sibling
		((ui_rect *)obj)->w = ((ui_rect *)obj->parent)->w - h;
	} else {
		//autofill parent
		((ui_rect *)obj)->w = ((ui_rect *)obj->parent)->w;
	}
	w = ((ui_rect *)obj)->w;
	xx = 0;
	memset(&mold, 0, sizeof(ui_object));
	mold.backcolor = obj->backcolor;
	((ui_rect *)&mold)->w = 16;
	((ui_rect *)&mold)->h = 16;
	((ui_rect *)&mold)->y = obj->base.y;
	xx += 2;
	((ui_infostat *)obj)->tick--;
	if(obj->state & UI_STATE_INIT) {
		//reset infostat background
		((ui_infostat *)obj)->tick = 0;
		display->fill_area(display, obj->backcolor, display->set_area(display, x, y, w, 16));
	}
	percent = os_get_cpu_util();
	if(obj->state == UI_STATE_INIT) {
		((ui_rect *)&mold)->x = (x + xx);
		ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_cpu16, sizeof(image_png_cpu16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		
		((ui_rect *)&mold)->x = (x + xx) + 48;
		ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_lcd16, sizeof(image_png_lcd16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
	}
	
	if((((ui_infostat *)obj)->tick % 10) == 0) {
		//if((obj->state & UI_STATE_INIT) == 0) {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx + 16, y, 32, 16));
		//}
		((ui_infostat *)obj)->cpu_util = (100 - percent);
		sprintf((char *)linebuf, "%d%%", ((ui_infostat *)obj)->cpu_util);
		display->print_string(display, UI_FONT_SMALL, x + xx + 18, y, linebuf, ((ui_object *)obj)->forecolor);
		sprintf((char *)linebuf, "%d%%", os_heap_usage());
		display->print_string(display, UI_FONT_SMALL, x + xx + 18, y+8, linebuf, ((ui_object *)obj)->forecolor);
		
		xx = 48;
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx + 16, y, 40, 16));
		sprintf((char *)linebuf, "%0.1fFPS", ((ui_infostat *)obj)->fps);
		display->print_string(display, UI_FONT_SMALL, x + xx + 18, y, linebuf, ((ui_object *)obj)->forecolor);
	}
	//network type/activity indicator
	xx = 96;
	((ui_rect *)&mold)->x = (x + xx);
	if((((ui_infostat *)obj)->tick % 6) == 0) {
		if(ctx->netctx->state & IF_NET_STATE_BUSY) {
			switch((((ui_infostat *)obj)->tick % 24) / 4) {
				case 0:
					ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_netbusy_2, sizeof(image_png_netbusy_2), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
					break;
				case 1:
					ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_netbusy_1, sizeof(image_png_netbusy_1), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
					break;
				default:
				case 2:
					ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_netbusy_0, sizeof(image_png_netbusy_0), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
					break;
			}		
		} else {
			//network idle
			switch(ctx->netctx->type) {
				case NET_TYPE_WIFI:
					ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_wifi16, sizeof(image_png_wifi16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
					break;
				case NET_TYPE_GSM:
					ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_gprs16, sizeof(image_png_gprs16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
					break;
				default: 
					break;
			}
		}
	}
	xx += 18;
	//battery status;
	if(xx > (w - 16)) goto exit_infostat_render;
	signal = (ui_signal *)ui_get_object_by_name(ctx->display, "xsignal") ;
	if(signal != NULL) {
		if(if_pwr_is_charging()) {
			((ui_rect *)&mold)->x = (x + xx);
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_batt_plugged16, sizeof(image_png_batt_plugged16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		} else {
			((ui_rect *)&mold)->x = (x + xx);
			if(signal->batt < 20) {
				ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_batt_low16, sizeof(image_png_batt_low16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
			} else {
				display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y, 16, 16));
			}
		}
		xx += 18;
	}
	//usb status
	if(xx > (w - 16)) goto exit_infostat_render;
	if(ctx->uctx->instance->dev_state != USBD_STATE_DEFAULT) {
		((ui_rect *)&mold)->x = (x + xx);
		if(ctx->uctx->instance->dev_state != USBD_STATE_SUSPENDED) {		//usb connected
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_usb16, sizeof(image_png_usb16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		} else {																					//usb ready for connection
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_usb_ready16, sizeof(image_png_usb_ready16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		}
	} else {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y, 16, 16));
	}
	xx += 18;
	//card status
	if(xx > (w - 16)) goto exit_infostat_render;
	if(if_card_state(ctx->cctx) == 0) {			//card inserted state
		//display->print_string(display, UI_FONT_DEFAULT, x + xx, y, (uint8 *)"R", ((ui_object *)obj)->forecolor);
		((ui_rect *)&mold)->x = (x + xx);
		ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_card16, sizeof(image_png_card16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
	} else {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y, 16, 16));
	}
	xx += 18;
	//autoplay status
	if(xx > (w - 16)) goto exit_infostat_render;
	if(ctx->runstate & TK_AUTOPLAY_ENABLED) {
		((ui_rect *)&mold)->x = (x + xx);
		if(ctx->runstate & TK_AUTOPLAY_RUN) {
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_cassete_play16, sizeof(image_png_cassete_play16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		} else {
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_cassete16, sizeof(image_png_cassete16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		}
	} else {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y, 16, 16));
	}
	xx += 18;
	//virtual machine status
	if(xx > (w - 16)) goto exit_infostat_render;
	if(ctx->cos_status & TK_COS_STAT_VM_STARTED) {			//Orb-Weaver running status
		//display->print_string(display, UI_FONT_DEFAULT, x + xx, y, (uint8 *)"R", ((ui_object *)obj)->forecolor);
		((ui_rect *)&mold)->x = (x + xx);
		ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_vm16, sizeof(image_png_vm16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		//ctx->cos_status &= ~TK_COS_STAT_VM_STARTED;		//remove status flag
	} else {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y, 16, 16));
	}
	xx += 18;
	//NFC status
	if(xx > (w - 16)) goto exit_infostat_render;
	if(ctx->rctx->nfc_state & NFC_STATE_INITIALIZED) {
		((ui_rect *)&mold)->x = (x + xx);
		if(ctx->rctx->nfc_state & NFC_STATE_CONNECTED) {
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_nfc_connected16, sizeof(image_png_nfc_connected16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		} else {
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_nfc_ready16, sizeof(image_png_nfc_ready16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		}
	} else {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y, 16, 16));
	}
	xx += 18;
	//bluetooth status
	if(xx > (w - 16)) goto exit_infostat_render;
	if(ctx->bctx->state & BLE_STATE_INITIALIZED) {
		((ui_rect *)&mold)->x = (x + xx);
		if(ctx->bctx->state & BLE_STATE_CONNECTED) {
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_ble_connected16, sizeof(image_png_ble_connected16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		} else {
			ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_ble_ready16, sizeof(image_png_ble_ready16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		}
	} else {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y, 16, 16));
	}
	xx += 18;
	//update status
	if(xx > (w - 16)) goto exit_infostat_render;
	if(ctx->cos_status & TK_COS_STAT_UPDATE_AVAIL) {
		((ui_rect *)&mold)->x = (x + xx);
		ui_image_render(display, (ui_rect *)&mold, (uint8 *)image_png_download16, sizeof(image_png_download16), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
	} else {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y, 16, 16));
	}
	xx += 18;
	
	//freespace gauge
#if 0
	if(if_card_state(ctx->cctx) == 0) {
		xx = w - (w / 4);
		percent = (ctx->cos_freespace * 100 )/ ctx->cos_totalspace;
		percent = 100 - percent;
		//draw border
		display->draw_rectangle(display, x + xx + 1, y+3, wgauge-2, 8, UI_COLOR_WHITE, 1);
		//draw content
		display->set_area(display, x + xx + 3, y + 5, wgauge - 6, 8);
		xlen = wgauge  - 6;
		x1 = (percent * xlen) / 100;
		//display->fill_area(display, UI_COLOR_GREY, display->set_area(display, x+3, y+3, xlen, h-6));
		display->fill_area(display, UI_COLOR_SLATE_GREY, display->set_area(display, x + xx + 3, y + 5, x1, 4));
		display->fill_area(display, obj->backcolor, display->set_area(display, x+ xx + 3 + x1, y + 5, (wgauge - 6) - x1, 4));
	} else {
		display->fill_area(display, obj->backcolor, display->set_area(display, x + xx, y + 3, wgauge, 8));
	}
		xx += wgauge;
	//current active devlet
	//}
#endif
	exit_infostat_render:
	//print console message
	if(((ui_rect *)obj)->h >= 24 && ((ui_infostat *)obj)->tick == 0) {
		xx = 2;
		//clear textarea
		display->fill_area(display, obj->backcolor, display->set_area(display, x, y + 16, w, 8));
		//print text if any
		if(strlen(((ui_text *)obj)->text) != 0) display->print_string(display, UI_FONT_SMALL, x + xx, y + 16, (uint8 *)((ui_text *)obj)->text, ((ui_object *)obj)->forecolor);
		else if(ctx->config->devlet_name[0] < 0x80) display->print_string(display, UI_FONT_SMALL, x + xx, y + 16, ctx->config->devlet_name, ((ui_object *)obj)->forecolor);
		((ui_infostat *)obj)->tick = 30;		//2s
	}
	OS_DEBUG_EXIT();
}

extern void dashboard_init(gui_handle_p handle, void * params);

void tk_init_gui(tk_context_p handle) {
	OS_DEBUG_ENTRY(tk_init_gui);
	ui_object * stat = NULL;
	ui_object * card = NULL;
	ui_object * signal = NULL;
	ui_object * header;
	uint16 header_w ;
	uint16 meta_h;
	uint8 batt_percent = 100;
	gui_handle_p display = (gui_handle_p)handle->display;
	header = display->header;
	if(header != NULL) { 
		header_w = header->rect.w;
		//batt_percent = if_pwr_get_batt_percent();
		ui_add_header(handle->display, (signal = ui_signal_create((uint8 *)"xsignal", header_w, header_w, 0, batt_percent, handle->netctx)));
		ui_add_header(handle->display, (card = ui_card_create((uint8 *)"myCard", (uint8 *)"Home", header_w, header_w, tk_card_click)));
		if(card != NULL) ui_set_target(card, handle);
		//ui_add_header(handle->display, ui_action_create((uint8 *)"myAction", header_w, header_w));			//reserved space
		dashboard_init(handle->display, handle);
		tk_setting_init(handle->display, handle);
		if(display->width >= 240) {
			meta_h = 24;
		} else {
			meta_h = 16;
		}
		((gui_handle_p)handle->display)->meta = ui_create_panel(sizeof(ui_panel), header_w, 0, display->width - header_w, meta_h, (uint8 *)"status", 0, 0);
		ui_set_align(((gui_handle_p)handle->display)->meta, UI_ALIGN_FLOAT);
		ui_add_object(((gui_handle_p)handle->display)->meta, (stat = ui_infostat_create(handle)));
		if(meta_h >= 24) {
			ui_add_object(((gui_handle_p)handle->display)->meta, ui_tasker_create());
		}
		if(stat != NULL) {
			ui_set_object_name(stat, "infoBar");
			ui_set_align(stat, UI_ALIGN_HORIZONTAL);
			stat->render = tk_infostat_render;
		}
		if(display->orientation & 0x01) {		//vertical orientation
			//meta region
			ui_set_size(((gui_handle_p)handle->display)->meta, display->width, meta_h);
			ui_set_position(((gui_handle_p)handle->display)->meta, 0, 0);
			//body region
			ui_set_size(((gui_handle_p)handle->display)->body, display->width, display->height - meta_h);
			ui_set_position(((gui_handle_p)handle->display)->body, 0, meta_h);
		} else {								//horizontal orientation
			//meta region
			ui_set_size(((gui_handle_p)handle->display)->meta, display->width - header_w, meta_h);
			ui_set_position(((gui_handle_p)handle->display)->meta, header_w, 0);
			//body region
			ui_set_size(((gui_handle_p)handle->display)->body, display->width - header_w, display->height - meta_h);
			ui_set_position(((gui_handle_p)handle->display)->body, header_w, meta_h);
		}
		tk_clear_body(handle);
	}
	OS_DEBUG_EXIT();
}

void tk_clear_body(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_clear_body);
	//ui_clear_dispstack(ctx->display);
	ui_clear_body(ctx->display, NULL);
	if(ctx->app_state & TK_APP_STATE_RUNNING) {
		ui_add_body(ctx->display, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, ctx->app_name));
	}
	OS_DEBUG_EXIT();
}

void tk_show_menu(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_show_menu);
	//show setup menu
	BYTE tlen;
	uint8 blen;
	BYTE i;
	BYTE tag;
	uint16 len = gb_registered_size;
	WORD size;
	ui_item * obj;
	BYTE tbuf[70];
	tk_clear_body(ctx);
	if((tlen = tk_pop_by_tag(gba_registered_menu, len, STK_TAG_ALPHA, tbuf)) != 255) {
		tbuf[tlen] = 0;
		ui_add_body(ctx->display, ui_label_create(UI_COLOR_WHITE, 1, UI_FONT_DEFAULT, tbuf));
	}
	for(i =0; i<len; ) {
		i += tk_pop(gba_registered_menu + i, &tag, &size, tbuf);
		if((tag &0x7F) == STK_TAG_ITEM) {
			tbuf[size] = 0;
			ui_add_body(ctx->display, (obj = (ui_item *)ui_item_create(tbuf + 1, NULL, 0, tbuf[0], tk_menu_item_click)));
		}
	}	
	OS_DEBUG_EXIT();
}


static tk_resource_record g_ptr_resources[SHARD_MAX_RESOURCES];
uint8 g_idx_res_id = 0;

void tk_clear_resources(tk_context_p ctx) {
	uint8 i;
	g_idx_res_id = 0;
	//free up resource memory
	for(i=0;i<SHARD_MAX_RESOURCES;i++) {
		if(g_ptr_resources[i].res_size != NULL) os_free(g_ptr_resources[i].res_ptr);
	}
	//clear resources pointers
	memset(g_ptr_resources, 0, sizeof(g_ptr_resources));
}

uint8 tk_add_resources(tk_context_p ctx, uint16 len, uint8 * buffer) {
	uint8 * resource = NULL;
	uint8 idx;
	if(g_idx_res_id >= SHARD_MAX_RESOURCES) return 0;			//no available memory
	resource = os_alloc(len);
	if(resource != NULL) {
		memcpy(resource, buffer, len);
		idx = g_idx_res_id++;
		g_ptr_resources[idx].res_ptr = resource;
		g_ptr_resources[idx].res_size = len;
	}
	return g_idx_res_id;
}

tk_resource_record_p tk_get_resource(tk_context_p ctx, uint8 id) {
	if(id == 0) return NULL;
	if(id > SHARD_MAX_RESOURCES) return NULL;
	return &g_ptr_resources[id - 1];
}

uint8 tk_load_resource(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags) {		//return length of resource_id and resource_id (byref tags)
	OS_DEBUG_ENTRY(tk_load_resource);
	uint8 tlen;
	uint8 blen;
	uint8 i;
	uint8 ret = 0;
	uint8 tag;
	uint16 res_size = 0;
	ui_item * obj;
	ui_alert_p alert = NULL;
	net_protocol_p netp;
	uint16 certsz = 0;
	ssl_cert_p cert = NULL;
	net_request req;
	uint8 tbuf[240];
	uint8 hbuf[255];
	tk_clear_body(ctx);
	switch(mode & 0x1F) {
		case 0x00:		//from card
			break;
		case 0x01:		//from URI
			if((tlen = tk_pop_by_tag(gba_registered_menu, len, STK_TAG_URL, hbuf)) != (uint8)-1) {
				//load information text if available
				if((tlen = tk_pop_by_tag(gba_registered_menu, len, STK_TAG_ALPHA, tbuf)) != (uint8)-1) {
					tbuf[tlen] = 0;
				} else 
					tbuf[0] = 0;
				start_send_request:
				if_net_wake(ctx->netctx);
				netp = net_get_protocol(hbuf);		//get underlying application protocol by URI, http, coap, etc...
				if(netp != NULL) {
					//mode[0:2]  	=> 8 type of request methods (HEADER,GET,POST,PUT,DELETE,....)
					//mode[3]  	=> transport protocol (1=UDP,0=TCP)
					if(netp->type & NETP_USE_SSL) {
						alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)tbuf, UI_ALERT_INFO, UI_ALERT_ICON_SECURE);
					} else {
						alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)tbuf, UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
					}
					if(len != 0) {
						//decode http/coap parameters list Key-Value list
						len = tk_decode_params(tags, tbuf);
					}
					//if SSL used, initialize certificate and ssl context
#if SHARD_SSL_SUPPORT
					if(netp->type & NETP_USE_SSL) {
						//load certificate from card
						certsz = tk_load_card_certificate(ctx, gba_net_buffer);	//--> this function use apdu_buffer
						tk_pop(gba_net_buffer, &tag, &certsz, gba_net_buffer);
						if(tag != 0x9C) goto exit_send_request;
						//initialize secure socket layer
						//if(if_net_ssl_init(ctx->netctx, gba_net_buffer, certsz) != 0) goto exit_send_request;
						if((cert = if_ssl_create_cert(gba_net_buffer, certsz)) == NULL) goto exit_send_request;
					}
#endif
					//start request
					
					res_size = netp->send(ctx->netctx, net_request_struct(&req, mode, hbuf, netp->port, cert),
						NULL, tbuf, len, gba_net_buffer);
					
#if SHARD_SSL_SUPPORT
					if(netp->type & NETP_USE_SSL) {
						//release secure socket layer
						//if_net_ssl_release(ctx->netctx);
						if_ssl_release_cert(cert);
					}
#endif
				}
				exit_send_request:
				if_net_sleep(ctx->netctx);
				tags[0] = tk_add_resources(ctx, res_size, gba_net_buffer);
				ret = 1;		//sizeof resource id
			}
			break;
			
	}
	OS_DEBUG_EXIT();
	return ret;
}

uint8 tk_store_resource(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags) {	//return operation status
	return STK_RES_SUCCESS;
}