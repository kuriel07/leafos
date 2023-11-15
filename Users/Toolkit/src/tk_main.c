#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\..\ether\inc\owl.h"
#include "..\..\ether\inc\netbios.h"
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#include "..\..\orbweaver\inc\VMStackApis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\..\gui\inc\ui_resources.h"
#include "jansson.h"
#include "efat.h"

//uint16 gw_datafield_length;
uint8 gba_registered_menu[256];
uint8 gb_registered_size = 0;
uint8 gb_tk_state = TK_STATE_LIST_APP;
#define gba_apdu_data_field (gba_apdu_buffer + 5)  
extern uint8 gba_net_buffer[NET_BUFFER_SIZE];
uint16 gba_orc_size = 0;
uint8 gb_cmd_qualifier;
uint16 g_counter = 0;

static uint32_t FMC_Initialized = 0;


//static uint8 tk_list_app_for_delete(tk_context_p ctx) ;
static void tk_app_delete_click(ui_object * obj, void * params);

ui_object * tk_present(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_present);
	ui_object * ret;
#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON
	//try making a usb callback if exist
	//tk_usb_exec(ctx);
#endif
	gui_handle_p display = ctx->display;
#if defined(STM32F746xx)
	void * prevfb = display->fb_ptr;
	display->fb_ptr = os_alloc(display->fb_size);
	if(display->fb_ptr == NULL) {
		display->fb_ptr = prevfb;
	} else {
		//copy frame buffer
		#if 1
	dma_memcpy(display->fb_ptr, prevfb, display->fb_size);
	dma_wait();
	#endif
	//os_wait(200);
	//while(_xfer_completed == FALSE) {
	//	os_wait(15);
	//}
	//HAL_DMA_PollForTransfer(&dma_x, HAL_DMA_FULL_TRANSFER, 200);
		
		//memcpy(display->fb_ptr, prevfb, display->fb_size);
	}
#endif
	
	ret = (ui_object *)if_gui_present(ctx->display);
	
#if defined(STM32F746xx)
	if(prevfb != NULL && display->fb_ptr != NULL) os_free(prevfb);
#endif
	
#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON
	//tk_usb_callback(ctx, gba_apdu_data_field, 0);
#endif
	OS_DEBUG_EXIT();
	return ret;
}

static void tk_river_icon_click(ui_object * app, void * params) {
	OS_DEBUG_ENTRY(tk_river_icon_click);
	//select application during application list
	//ui_icon * icon = (ui_icon *)app;
	ui_app_item * icon = (ui_app_item *)app;
	tk_select_application(app->target, icon->aidlen, icon->aid) ;
	gb_tk_state = TK_STATE_RIVER;
	OS_DEBUG_EXIT();
}

static void tk_app_icon_click(ui_object * app, void * params) {
	OS_DEBUG_ENTRY(tk_app_icon_click);
	//select application during application list
	//ui_icon * icon = (ui_icon *)app;
	ui_app_item * icon = (ui_app_item *)app;
	tk_select_application(app->target, icon->aidlen, icon->aid) ;
	gb_tk_state = TK_STATE_LIST_MENU;
	OS_DEBUG_EXIT();
}

void tk_aid2text(BYTE * textbuffer, BYTE maxlen, BYTE * aid, BYTE aidlen) {
	OS_DEBUG_ENTRY(tk_load_cos_config);
	uint8 str_len = 0;
	uint8 i;
	uint8 vlen, alen;
	if(aid[0] == 0xAF) {			//card manager
		str_len = aid[1];
		memcpy(textbuffer, aid + 2, str_len);
		textbuffer[str_len] = 0;
		goto exit_aid_to_text;
	} else {
		i = 2;
		vlen = ((aid[1] >> 4) & 0x7) ;
		alen = aid[1] & 0x07;
		i += vlen;
		str_len = (vlen + alen + 1);
		memcpy(textbuffer, aid + 2, vlen);
		textbuffer[vlen] = '.';
		memcpy(textbuffer + vlen + 1, aid + i, alen);
		textbuffer[str_len] = 0;
	}
	goto exit_aid_to_text;
	if(str_len >= maxlen) goto exit_aid_to_text;
	textbuffer[str_len++] = '[';
	for(i=0;i<aidlen && str_len < (maxlen -3) && i<8;i++) {
		sprintf((char *)textbuffer + str_len, "%02x ", aid[i]);
		str_len += 3;
	}
	textbuffer[str_len-1] = ']';
	textbuffer[str_len] = 0;
	exit_aid_to_text:
	OS_DEBUG_EXIT();
	return;
}

void tk_load_cardconfig(tk_context_p ctx, uint8 * config, uint16 len) {
	OS_DEBUG_ENTRY(tk_load_cos_config);
	uint16 i,j;
	uint16 size;
	uint8 tag;
	uint8 namelen = 0;
	//uint32 grid;
	for(i=0; i<len;) {
		i += tk_pop(config + i, &tag, &size, config);
		switch(tag) {
			case 0x31:		//GRID
				ctx->cos_grid = end_swap32(*(uint32 *)config);
				break;
			case 0x8C:		//card configuration (8 bytes minimum), perso configuration
				
				break;
			case 0x03:		//card alias
				
				break;
		}	
	}
	OS_DEBUG_EXIT();
}

void tk_load_cos_config(tk_context_p ctx, uint8 * config, uint16 len) {
	OS_DEBUG_ENTRY(tk_load_cos_config);
	uint16 i,j;
	uint16 size;
	uint8 tag;
	uint8 namelen = 0;
	uint32 space;
	//clear current configuration
	ctx->cos_freespace = 0;
	ctx->cos_totalspace = 0;
	ctx->cos_owver = 0x00;
	memset(ctx->cos_config, 0, TK_COS_CONFIG_SIZE);
	for(i=0; i<len;) {
		i += tk_pop(config + i, &tag, &size, config);
		switch(tag) {
			case 0xFE:		//freespace (file system emtpy)
				space = end_swap32(*(uint32 *)config);
				ctx->cos_freespace = space;
				break;
			case 0xF0:		//file system size (file system initial)
				space = end_swap32(*(uint32 *)config);
				ctx->cos_totalspace = space;
				break;
			case 0x80:
				ctx->cos_owver = config[9];		//clCoreDebugInfo (orbweaver version)
				break;
			case 0x87:		//orb-weaver card configuration
				memcpy(ctx->cos_config, config, 4);
				break;
			case 0xFA:		//current card lifecycle
				if((config[0] & 0x05) != 0x05) {					//check for activated state
					if((config[0] & 0x0C) == 0x0C) break;		//check for terminated state
					//only dormant and disabled could make a request for activation
					ctx->flag |= TK_FLAG_CARD_DORMANT;		//set card dormant flag
				}
				break;
		}	
	}
	OS_DEBUG_EXIT();
}

uint8_c g_baShardConfig[] = { SHARD_TERMINAL_CONFIG, SHARD_TERMINAL_INPUT, SHARD_TERMINAL_OUTPUT, 0, 0, 0, 0 };
uint8_c * g_strErrConfText[] = {
	(uint8_c *)"None",
	(uint8_c *)"Invalid API Key",
	(uint8_c *)"Unsupported Framework"
};

static uint8 tk_load_appconfig(tk_context_p tctx, uint8 * config, uint16 len) {
	OS_DEBUG_ENTRY(tk_load_appconfig);
	uint16 i,j;
	uint16 size;
	uint8 tag;
	uint8 ret;
	uint8 namelen = 0;
	uint16 build_number = 0;
	//uint8 	owver; 								//minimum orbweaver version
	//uint8 	ver;								//application version
 	//uint8 	api_level;							//current api configuration
	//uint8 	security;							//security requirement
	//uint8 	config;								//card-terminal configuration
	//uint16 build_number					//build number (big endian)
	tk_pop(config, &tag, &size, config);
	if(tag != 0xAC) { ret = 1; goto exit_load_appconfig; }			//invalid API key
	if(config[0] > 0x14) { ret = 2; goto exit_load_appconfig; }		//unsupported framework
	if(config[3] & 0x10) tctx->flag |= TK_FLAG_SECURE;
	build_number = ((uint16)config[5] << 8) | config[6];			//build number
	tctx->app_bnum = build_number;
	ret = 0;
	exit_load_appconfig:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 tk_select_application(tk_context_p tctx, uint8 len, uint8 * aid) {
	OS_DEBUG_ENTRY(tk_select_application);
	uint16 status;
	uint16 rlen;
	uint8 err = -1;
	tctx->flag = TK_FLAG_SECURE;			//reset toolkit flag during application selection
	status = tk_card_send(tctx, gba_apdu_buffer, tk_build_apdu_data(gba_apdu_buffer, 0x00, 0xA4, 0x04, 0x00, len, aid), gba_apdu_data_field, &rlen);
	if(status == 0x9000) {
		//File Control Information
		//selecting Orb-River
		if(aid[0] == 0xAF) {			//card manager
			//gb_tk_state = TK_STATE_RIVER;
		} else {
			tk_push_action(tctx, 0x4F, aid, len);
			//save current selected AID to context
			tctx->app_state = TK_APP_STATE_RUNNING;
			tk_aid2text(tctx->app_name, TK_MAX_AIDLEN, aid, len);
			//check orblet application configuration
			status = tk_card_send(tctx, gba_apdu_buffer, tk_build_apdu(gba_apdu_buffer, 0x00, 0xCA, 0x00, 0xAC, 0x00), gba_apdu_buffer, &rlen);
			if(status == 0x9000 || status == 0x62F1) {
				//validate application configuration
				if((err = tk_load_appconfig(tctx, gba_apdu_buffer, rlen)) != 0) goto exit_select_application;
			}
		}
		err = 0;
		goto exit_select_application;
	}
	err = -1;
	exit_select_application:
	OS_DEBUG_EXIT();
	return err;
}

static void tk_delete_application(tk_context_p ctx, uint8 aidlen, uint8 * aid) {
	OS_DEBUG_ENTRY(tk_delete_application);
	ui_alert * alert;
	uint8 fcpField[32];
	uint8 datField[270];		//use local buffer as temporary C_APDU/R_APDU
	uint16 len;
	uint16 status;
	len = tk_push(fcpField, 0x4F, aidlen, aid);
	alert = ui_alert_show(ctx->display, (uint8 *)"Deleting", (uint8 *)"Please Wait...", UI_ALERT_INFO, UI_ALERT_ICON_INFO);
	status = tk_card_send(ctx, datField, tk_build_apdu_data(datField, 0x80, 0xE4, 0x00, 0x80, len, (uint8 *)fcpField), datField, &len);
	ui_alert_close(ctx->display, alert);
	OS_DEBUG_EXIT();
}

uint8 tk_confirm_delete(tk_context_p ctx, uint8 aidlen, uint8 * aid) {
	OS_DEBUG_ENTRY(tk_confirm_delete);
	ui_alert * alert;
	ui_object * obj;
	uint8 datField[270];		//use local buffer as temporary C_APDU/R_APDU
	uint8 fcpField[32];
	uint16 len;
	uint16 status;
	uint8 ret = -1;
	alert = ui_alert_show(ctx->display, (uint8 *)"Confirmation", (uint8 *)"Delete", UI_ALERT_BUTTON_OK | UI_ALERT_BUTTON_CANCEL, UI_ALERT_ICON_INFO);
	gb_tk_state = TK_STATE_IDLE;
	//start ui rendering presentation (enter loop)
#if SHARD_RTOS_ENABLED
	obj = tk_wait_user_input(ctx);
#else
	while(gb_tk_state == TK_STATE_IDLE) {
		obj = tk_present(ctx);
		if(if_card_state(ctx->cctx) != 0)  gb_tk_state = TK_STATE_CARD_DISCONNECTED;
	}
#endif
	ui_alert_close(ctx->display, alert);
	if(gb_tk_state != TK_STATE_CARD_DISCONNECTED) {
		if(ui_alert_result(ctx->display) == UI_ALERT_RESULT_OK) {
			//tk_list_app_for_delete(ctx);
			tk_delete_application(ctx, aidlen, aid);
		}
		ret = 0;
	} 
	OS_DEBUG_EXIT();
	return ret;
}

static uint16 tk_load_icon(tk_context_p ctx, uint8 aidlen, uint8 * aid, uint8 * iconbuf) {
	uint16 len;
	uint16 status;
	uint16 icolen = 0;
	status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu_data(gba_apdu_buffer, 0x00, 0xA4, 0x04, 0x00, aidlen, aid), gba_apdu_buffer, &len);
	if(status != 0x9000) return 0;
	status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu_data(gba_apdu_buffer, 0x00, 0xCA, 0x4F, 0x83, 0x01, (uint8 *)"\x99"), gba_apdu_buffer, &len);
	if(status == 0x9000 || status == 0x62F1) {
		memcpy(iconbuf + icolen, gba_apdu_buffer, len);
		icolen += len;
	} else return 0;
	//fetch next data chunk
	while(status == 0x62F1) {
		status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu(gba_apdu_buffer, 0x00, 0xCA, 0x4F, 0x00, 0x00), gba_apdu_buffer, &len);
		//if(status != 0x62F1) break;
		memcpy(iconbuf + icolen, gba_apdu_buffer, len);
		icolen += len;
	}
	return icolen;
}

uint8 tk_list_application(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_list_application);
	ui_object * obj;
	uint8 datField[270];		//use local buffer as temporary C_APDU/R_APDU
	uint8 wBuffer[256];
	uint8 wLen;
	uint16 status;
	uint8 i = 0, j;
	uint8 str_buffer[70];
	uint8 aid[16];
	uint8 aidlen;
	uint8 waid[24];
	uint16 waidlen;
	uint8 tag;
	ui_object * app_obj;
	uint16 len;
	uint16 size;
	uint8 * icoptr;
	uint16 icolen;
	uint8 percent;
	ui_alert * alert;
	uint8 err = -1;
	ui_list_scroll * listbox;
	ui_object * body;
	aidlen = tk_pop_action(ctx, &tag, aid);
	if(tag != 0x4F) aidlen = 0;
	//load card configuration
	status = tk_card_send(ctx, datField, tk_build_apdu(datField, 0x00, 0xCA, 0x00, 0xCF, 0x00), datField, &len);
	if(status == 0x9000) {
		//load card configuration
		tk_pop(datField, &tag, &len, datField);
		if(tag == 0xCF) {
			tk_load_cardconfig(ctx, datField, len);
		}
	}
	//load cos configuration (debug info)
	status = tk_card_send(ctx, datField, tk_build_apdu(datField, 0x00, 0xCA, 0x00, 0xDB, 0x00), datField, &len);
	if(status == 0x9000) {
		tk_pop(datField, &tag, &len, datField);
		if(tag == 0xDB) {
			tk_load_cos_config(ctx, datField, len);
		}
	}
	status = tk_card_send(ctx, datField, tk_build_apdu_data(datField, 0x00, 0xCA, 0x2F, 0x00, 0x01, (uint8 *)"\x5C"), datField, &len);
	tk_clear_body(ctx);
	body = ((gui_handle_p)ctx->display)->body;
	if(status == 0x9000) {
		for(i=0; i<len;) {
				i += tk_pop(datField + i, &tag, &size, waid);
				if(tag != 0x61) continue;
				tk_pop(waid, &tag, &waidlen, waid);
				if(tag != 0x4F) continue;
				if(waid[0] == 0xAF) {
					//save current card manager
					memcpy(ctx->cmaid, waid, waidlen);						//save current AID as default card manager
					ctx->cmlen = waidlen;
				}
		}
	}
	if(status == 0x9000) {
		if(len > 256) {
			err = -1;
			goto exit_list_application;
		}
		memcpy(wBuffer, datField, len);
		wLen = len;
		//check for matched autoplay AID
		for(i=0; i<len && aidlen != 0;) {
			i += tk_pop(datField + i, &tag, &size, waid);
			if(tag != 0x61) continue;
			tk_pop(waid, &tag, &waidlen, waid);
			if(tag != 0x4F) continue;
			if(waidlen == aidlen && memcmp(waid, aid, aidlen) == 0) {
				if((err = tk_select_application(ctx, aidlen, aid)) != (uint8)0) {
					gb_tk_state = TK_STATE_CONFIG_ERROR;
					err = -1;
				} else {
					gb_tk_state = TK_STATE_LIST_MENU;
					err = 0;
				}
				goto exit_list_application;
			}
		}
		
#if 0
		alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Loading...", UI_ALERT_PROGRESS_BAR, UI_ALERT_ICON_INFO);
		for(i=0; i<len;) {
			i += tk_pop(datField + i, &tag, &size, datField);
			if(tag != 0x61) continue;
			tk_pop(datField, &tag, &size, datField);
			if(tag != 0x4F) continue;
			tk_aid2text(str_buffer, sizeof(str_buffer), datField, size);
			//load icon file from selected orblet application
			icolen = tk_load_icon(ctx, size, datField, gba_net_buffer);
			if(icolen != 0) {
				tk_pop(gba_net_buffer, &tag, &icolen, gba_net_buffer);
			} else {
				if(datField[0] == 0xAF) {		//card manager
					icolen = sizeof(gba_river_icon);
					memcpy(gba_net_buffer, gba_river_icon, icolen);
				} else {							//other executable
					icolen = sizeof(gba_default_icon);
					memcpy(gba_net_buffer, gba_default_icon, icolen);
					//ui_add_object(body, (ui_object *)(app_obj = (ui_icon *)ui_icon_create(str_buffer, size, datField, icolen, gba_net_buffer, tk_app_icon_click)));
					//((ui_object *)app_obj)->target = ctx;
				}
			}
			if(datField[0] == 0xAF) {		//card manager
				ui_add_object(body, (ui_object *)(app_obj = (ui_icon *)ui_icon_create(str_buffer, size, datField, icolen, gba_net_buffer, tk_river_icon_click)));
			} else {
				ui_add_object(body, (ui_object *)(app_obj = (ui_icon *)ui_icon_create(str_buffer, size, datField, icolen, gba_net_buffer, tk_app_icon_click)));
			}
			((ui_object *)app_obj)->target = ctx;
			ui_alert_set_progress_bar(ctx->display, alert, ((i * 100) /len));
			app_obj->aidlen = size;
		}
		ui_alert_close(ctx->display, alert);
#else
		listbox = (ui_list_scroll *)ui_list_create(0, 16, ((ui_rect *)body)->w, ((ui_rect *)body)->h - 16);
		ui_add_body(ctx->display, listbox);
		
		for(i=0; i<len;) {
			i += tk_pop(datField + i, &tag, &size, datField);
			if(tag != 0x61) continue;
			tk_pop(datField, &tag, &size, datField);
			if(tag != 0x4F) continue;
			tk_aid2text(str_buffer, sizeof(str_buffer), datField, size);
			if(datField[0] == 0xAF) {		//card manager
				ui_list_add((ui_object *)listbox, (ui_object *)(app_obj = ui_app_item_create(str_buffer, size, datField, tk_river_icon_click)));
				icoptr = (uint8 *)image_png_river24;
				icolen = sizeof(image_png_river24);
			} else if(datField[0] == 0xAE) {
				ui_list_add((ui_object *)listbox, (ui_object *)(app_obj = ui_app_item_create(str_buffer, size, datField, tk_app_icon_click)));
				icoptr = (uint8 *)image_png_exe24;
				icolen = sizeof(image_png_exe24);
			} else {
				ui_list_add((ui_object *)listbox, (ui_object *)(app_obj = ui_app_item_create(str_buffer, size, datField, tk_app_icon_click)));
				icoptr = (uint8 *)image_png_elf24;
				icolen = sizeof(image_png_elf24);
			}
			if(app_obj != NULL){
				((ui_object *)app_obj)->image.buffer = (uint8 *)icoptr;
				((ui_object *)app_obj)->image.size = icolen;
				((ui_object *)app_obj)->target = ctx;
			}
		}
#endif
	}
#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON
	tk_usb_callback(ctx, wBuffer, wLen);	///coba dihilangkan
#endif
	//switch state to idle
	gb_tk_state = TK_STATE_IDLE;
	exit_list_application:
	OS_DEBUG_EXIT();
	return err;		//list done, no selected application
}

void tk_app_delete(tk_context_p ctx, uint8 * aid, uint8 aidlen) {
	OS_DEBUG_ENTRY(tk_app_delete);
	if(aidlen <= 0x10) {
		memcpy(ctx->aidbuf, aid, aidlen);
		ctx->aidlen = aidlen;
	}
	OS_DEBUG_EXIT();
}

static void tk_app_delete_click(ui_object * obj, void * params) {
	OS_DEBUG_ENTRY(tk_app_delete_click);
	tk_app_delete(obj->target, ((ui_app_item *)obj)->aid, ((ui_app_item *)obj)->aidlen);
	gb_tk_state = TK_STATE_DELETE_APP;
	OS_DEBUG_EXIT();
}

uint8 tk_list_app_for_delete(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_list_app_for_delete);
	ui_object * obj;
	uint8 datField[270];		//use local buffer as temporary C_APDU/R_APDU
	uint16 status;
	uint8 i = 0, j;
	uint8 str_buffer[70];
	uint8 tag;
	uint16 len;
	uint16 size;
	uint8 percent;
	ui_object * list;
	ui_object * body;
	uint8 ret = -1;
	gui_handle_p display = (gui_handle_p)ctx->display;
	if(tk_select_application(ctx, ctx->cmlen, ctx->cmaid) != 0) goto exit_list_app_for_delete;		//card manager cannot be selected
	//load card configuration
	status = tk_card_send(ctx, datField, tk_build_apdu(datField, 0x00, 0xCA, 0x00, 0xCF, 0x00), datField, &len);
	if(status == 0x9000) {
		//load configuration
		tk_pop(datField, &tag, &len, datField);
		if(tag == 0xCF) {
			tk_load_cardconfig(ctx, datField, len);
		}
	}
	//load cos configuration (debug info)
	status = tk_card_send(ctx, datField, tk_build_apdu(datField, 0x00, 0xCA, 0x00, 0xDB, 0x00), datField, &len);
	if(status == 0x9000) {
		tk_pop(datField, &tag, &len, datField);
		if(tag == 0xDB) {
			tk_load_cos_config(ctx, datField, len);
		}
	}
	status = tk_card_send(ctx, datField, tk_build_apdu_data(datField, 0x00, 0xCA, 0x2F, 0x00, 0x01, (uint8 *)"\x5C"), datField, &len);
	//tk_clear_body(ctx);
	body = ui_push_screen(ctx->display, NULL);
	//body = ((gui_handle_p)ctx->display)->body;
	//percent = (ctx->cos_freespace * 100) / ctx->cos_totalspace;
	//sprintf((char *)str_buffer, "Free %dKB of %dKB", ctx->cos_freespace/1024, ctx->cos_totalspace/1024);
	//ui_add_object(body, (ui_object *)ui_gauge_create(UI_COLOR_WHITE, percent, str_buffer));
	list = ui_list_create(0, 0, ((ui_rect *)display->body)->w, (((ui_rect *)display->body)->h) - 16);
	ui_add_body(display, list);
	if(status == 0x9000) {
		for(i=0; i<len;) {
			i += tk_pop(datField + i, &tag, &size, datField);
			if(tag != 0x61) continue;
			tk_pop(datField, &tag, &size, datField);
			if(tag != 0x4F) continue;
			if(datField[0] == 0xAF) continue;		//skip card manager
			tk_aid2text(str_buffer, sizeof(str_buffer), datField, size);
			ui_list_add(list, (obj  = ui_app_item_create((uint8 *)str_buffer, size, datField, tk_app_delete_click)));
			if(obj != NULL) obj->target = ctx;		//set object target to current context
		}
	}
	ret = 0;		//status OK
	exit_list_app_for_delete:
	OS_DEBUG_EXIT();
	return ret;
}

uint16 tk_response(BYTE status, BYTE len, BYTE * tlv) {
	OS_DEBUG_ENTRY(tk_response);
	WORD sw;
	BYTE terminal_response[7] = { (0x80 | STK_TAG_DEV_ID), 0x02, STK_DEV_ME, STK_DEV_SIM, (0x80|STK_TAG_RESULT), 0x01, 0x00};
	terminal_response[6] = status;
	if(len != 0 && tlv != NULL) {
		memcpy(gba_apdu_data_field + sizeof(terminal_response), tlv, len);
	}
	memcpy(gba_apdu_data_field, terminal_response, sizeof(terminal_response));
	OS_DEBUG_EXIT();
	return sizeof(terminal_response) + len;
}

BYTE tk_dispatch(tk_context_p ctx, uint16 len, uint8 * buffer) {
	OS_DEBUG_ENTRY(tk_dispatch);
	uint16 sw;
	uint16 rlen;
	uint8 ins = 0x14;
	//terminal response
	if((buffer[0] & 0xF0) == 0xD0) ins = 0xC2;
	if(ctx->flag & TK_FLAG_SECURE) {
		len = tk_wrap_data_buffer(ctx, buffer, len);
	}
	len = tk_build_apdu_data(gba_apdu_buffer, 0x00, ins, 0x00, 0x00, len, buffer);
	sw = tk_card_send(ctx, gba_apdu_buffer, len, buffer, &rlen);
	if((sw & 0xF000) != 0x9000) rlen = 0;
	OS_DEBUG_EXIT();
	return rlen;
}

uint8 tk_setup_menu(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_setup_menu);
	uint16 status;
	uint16 len = 0;
	uint8 terminal_profile[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	//terminal profile
	memcpy(gba_apdu_data_field, terminal_profile, len = sizeof(terminal_profile));
	if(ctx->flag & TK_FLAG_SECURE) {
		len = tk_wrap_data_buffer(ctx, gba_apdu_data_field, len);
	}
	status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu_data(gba_apdu_buffer, 0x00, 0x10, 0x00, 0x00, len, gba_apdu_data_field), gba_apdu_data_field, &len);
	if(len != (uint16)-1) {
#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON
		tk_usb_callback(ctx, gba_apdu_data_field, len);
#endif
		if(status != 0x9000) {
			//no toolkit proactive command available (should show some error message)
			gb_tk_state = TK_STATE_IDLE;
			len = 0;
		}
	}
	OS_DEBUG_EXIT();
	return len;
}


void tk_push_action(tk_context_p tctx, uint8 tag, uint8 * buffer, uint8 length) {
	OS_DEBUG_ENTRY(tk_push_action);
	uint8 tbuffer[256];
	if((tctx->runstate & 0x7F) == TK_AUTOPLAY_RECORD) {
		if(tctx->offset > SHARD_AUTOREC_END) goto exit_push_action;			//exceeding record limit
		tbuffer[0] = tag;
		tbuffer[1] = length;
		memcpy(tbuffer + 2, buffer, length);
		tbuffer[length + 2] = 0xFF;
		if_flash_data_write(NULL, tctx->offset, tbuffer, length + 3) ;
		tctx->offset += (length + 2);
	}
	exit_push_action:
	OS_DEBUG_EXIT();
}

uint8 tk_pop_action(tk_context_p tctx, uint8 * tag, uint8 * buffer) {
	OS_DEBUG_ENTRY(tk_pop_action);
	uint8 ret = 0;
	uint8 tbuffer[2];
	if(tctx->runstate != (TK_AUTOPLAY_RUN | TK_AUTOPLAY_ENABLED)) goto exit_pop_action;
	if_flash_data_read(NULL, tctx->offset, tbuffer, 2);
	if(tbuffer[0] == 0xFF) { 
		tctx->runstate &= TK_AUTOPLAY_ENABLED;		//disable autorun
		goto exit_pop_action;		//no data available
	}
	tag[0] = tbuffer[0];
	if_flash_data_read(NULL, tctx->offset + 2, buffer, tbuffer[1]);
	tctx->offset += (tbuffer[1] + 2);
	ret = tbuffer[1];
	exit_pop_action:
	OS_DEBUG_EXIT();
	return ret;
}

//try auto execute menu based on stored setting
static uint8 tk_autoplay_menu(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_autoplay_menu);
	uint8 blen;
	BYTE i;
	uint8 len;
	BYTE tag;
	BYTE tbuf[69];
	BYTE mbuf[70];
	WORD size;
	uint8 ret = 0;
	gb_tk_state = TK_STATE_IDLE;
	if((blen = tk_pop_action(ctx, &tag, tbuf)) != 0) {
		if(tag != 0x25) goto exit_autoplay_menu;
		//check for matching menu name
		for(i =0; i<gb_registered_size; ) {
			i += tk_pop(gba_registered_menu + i, &tag, &size, mbuf);
			if((tag & 0x7F) == STK_TAG_ITEM) {
				mbuf[size] = 0;
				if(memcmp(mbuf+1, tbuf, blen) == 0) {
					len = tk_response(STK_RES_SUCCESS, 0, gba_apdu_data_field);
					len += tk_push(gba_apdu_data_field + len, STK_TAG_ITEM_ID, 1, mbuf);
					//use envelope menu
					len = tk_push(gba_apdu_data_field, ENV_TAG_MENU, len, gba_apdu_data_field);
					gb_tk_state = TK_STATE_EXECUTE;
					ret = tk_dispatch(ctx, len, gba_apdu_data_field);
					goto exit_autoplay_menu;
				}
			}
		}	
	}
	exit_autoplay_menu:
	OS_DEBUG_EXIT();
	return ret;
}

//decode from OWB object
uint16 tk_decode_params(uint8 * owb, uint8 * outbuf) {
	OS_DEBUG_ENTRY(tk_decode_params);
	uint16 i,j,k;
	uint16 wsize;
	uint8 tag;
	uint16 len;
	uint8 lbuf[240];
	//decapsulate array object
	tk_pop(owb, &tag, &wsize, owb);
	len = wsize;		
	switch(tag) {
		case OWB_TAG_SET:
			for(i=0,j=0;i<len;) {
				i += tk_pop(owb + i, &tag, &wsize, (uint8 *)lbuf + j);
				if(tag == OWB_TAG_OBJDESC) {
					for(k=0;k<wsize;k++) {
						if(lbuf[j + k] == ':') { 
							lbuf[j + k]= '=';			//replace colon with equal sign
							lbuf[j + wsize] = 0;	//put end string before encode to URI string
							net_get_uri_string((uint8 *)lbuf + j + k + 1, gba_net_buffer);
							strcpy((char *)lbuf + j + k + 1, (const char *)gba_net_buffer);
							strcat((char *)lbuf + j, (const char *)"&");
							j += strlen((const char *)lbuf + j);
							k = wsize;		//no need to parse next chars
						}
					}
				}
			}
			if(j != 0) lbuf[j-1] = 0;			
			len = j;
			break;
		default: len = 0; break;		//invalid tag
	}
	memcpy(outbuf, lbuf, len);
	OS_DEBUG_EXIT();
	return len;
}

uint16 tk_load_card_certificate(tk_context_p ctx, uint8 * certbuf) {
	OS_DEBUG_ENTRY(tk_load_card_certificate);
	uint16 len;
	uint16 status;
	uint16 certlen = 0;
	uint8 apdu_buffer[263];
	status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu_data(gba_apdu_buffer, 0x00, 0xCA, 0x4F, 0xAD, 0x01, (uint8 *)"\x9C"), gba_apdu_buffer, &len);
	if(status == 0x9000 || status == 0x62F1) {
		memcpy(certbuf + certlen, gba_apdu_buffer, len);
		certlen += len;
	} else return 0;
	//fetch next data chunk
	while(status == 0x62F1) {
		status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu(gba_apdu_buffer, 0x00, 0xCA, 0x4F, 0x00, 0x00), gba_apdu_buffer, &len);
		//if(status != 0x62F1) break;
		memcpy(certbuf + certlen, gba_apdu_buffer, len);
		certlen += len;
	}
	OS_DEBUG_EXIT();
	return certlen;
}

uint16 tk_send_device(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags) {
	ui_item_object * obj;
	uint8 tlen;
	uint8 hlen;
	uint8 tbuf[255];
	uint8 hbuf[10];
	uint16 dev_address;
	uint16 certsz = 0;
	ui_alert_p alert = NULL;
	uint8 tag;
	device_descriptor ddev;
	gba_orc_size = 0;
	
	strcpy((char *)tbuf, (const char *)"Relaying...");
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_ALPHA, tbuf)) != 255) {
		tbuf[tlen] = 0;
	}
	//get device address (2 bytes )
	if((hlen = tk_pop_by_tag(tags, len, STK_TAG_ADDRESS, hbuf)) == 255) goto exit_send_device;
	hbuf[hlen] = 0;
	dev_address = (hbuf[0] << 8) | hbuf[1];
	//get payload
	hlen = len;
	if((len = tk_pop_by_tag(tags, hlen, STK_TAG_CHANNEL_DATA, tags)) != (uint8)-1) { mode |= IF_PAYLOAD_PLAIN; goto start_send_device; }	//asumsi get/head
	//no payload detected
	len = 0;
	start_send_device:
	memset(&ddev, 0, sizeof(device_descriptor));
	//try opening device
	if(owl_find(ctx->octx, dev_address, &ddev) == 0) goto exit_send_device;
	if(ddev.address == 0) goto exit_send_device;										//no address assigned
	//start transmiting
	gba_orc_size = owl_transmit(ctx->octx, ddev.address, tags, len, gba_net_buffer);
	//close device
	owl_close(ctx->octx, ddev.address);
	exit_send_device:
	if(alert != NULL) ui_alert_close(ctx->display, alert);
	return gba_orc_size;
}

uint16 tk_send_request(tk_context_p ctx, uint8 mode, uint8 len, uint8 * tags) {
	OS_DEBUG_ENTRY(tk_send_request);
	ui_item_object * obj;
	uint8 tlen;
	uint8 hlen;
	uint8 tbuf[255];
	uint8 hbuf[255];
	uint16 certsz = 0;
	ui_alert_p alert = NULL;
	uint8 tag;
	ssl_cert_p cert = NULL;
	net_request req;
	net_protocol_p netp;
	gba_orc_size = 0;
	strcpy((char *)tbuf, (const char *)"Requesting...");
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_ALPHA, tbuf)) != 255) {
		tbuf[tlen] = 0;
	}
	//get URL
	if((hlen = tk_pop_by_tag(tags, len, STK_TAG_URL, hbuf)) == 255) goto exit_send_request;
	hbuf[hlen] = 0;
	hlen = tkGsm2Utf8(hbuf, hlen, sizeof(hbuf));			//convert to UTF8
	hbuf[hlen] = 0;
	//get payload
	hlen = len;
	if((len = tk_pop_by_tag(tags, hlen, STK_TAG_CHANNEL_DATA, tags)) != (uint8)-1) { mode |= IF_PAYLOAD_PLAIN; goto start_send_request; }	//asumsi get/head
	if((len = tk_pop_by_tag(tags, hlen, STK_TAG_PAYLOAD_PLAIN, tags)) != (uint8)-1) { mode |= IF_PAYLOAD_PLAIN; goto start_send_request; }	//asumsi get/head
	if((len = tk_pop_by_tag(tags, hlen, STK_TAG_PAYLOAD_JSON, tags)) != (uint8)-1) { mode |= IF_PAYLOAD_JSON; goto start_send_request; }	//asumsi get/head
	if((len = tk_pop_by_tag(tags, hlen, STK_TAG_PAYLOAD_OWB, tags)) != (uint8)-1) { mode |= IF_PAYLOAD_OWB; goto start_send_request; }	//asumsi get/head
	//no payload detected
	len = 0;
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
#if SHARD_SSL_SUPPORT
		//if SSL used, initialize certificate and ssl context
		if(netp->type & NETP_USE_SSL) {
			//load certificate from card
			certsz = tk_load_card_certificate(ctx, gba_net_buffer);	//--> this function use apdu_buffer
			tk_pop(gba_net_buffer, &tag, &certsz, gba_net_buffer);
			if(tag != 0x9C) goto exit_send_request;
			//initialize secure socket layer
			//if(if_net_ssl_init(ctx->netctx, gba_net_buffer, certsz) != 0) goto exit_send_request;
			if((cert = if_ssl_create_cert(gba_net_buffer, certsz)) == NULL) goto exit_send_request;
		}
		//start request
		net_request_struct(&req, mode, hbuf, netp->port, cert);
		gba_orc_size = netp->send(ctx->netctx, &req, NULL, tbuf, len, gba_net_buffer);
		
		if(netp->type & NETP_USE_SSL) {
			//release secure socket layer
			//if_net_ssl_release(ctx->netctx);
			if_ssl_release_cert(cert);
		}
#endif
	}
	//}
	exit_send_request:
	if_net_sleep(ctx->netctx);
	if(alert != NULL) ui_alert_close(ctx->display, alert);
	OS_DEBUG_EXIT();
	return gba_orc_size;
} 

uint8 tk_printer_send(void * display, ext_context_p ectx, BYTE len, BYTE * tags) {
	ui_item_object * obj;
	uint8 status;
	BYTE tlen;
	BYTE tbuf[256];
	ui_alert_p alert;
	if((tlen = tk_pop_by_tag(tags, len, STK_TAG_ALPHA, tbuf)) != 255) {
		tbuf[tlen] = 0;
	} else {
		tbuf[0] = 0;
	}
	alert = ui_alert_show(display, (uint8 *)"Information", (uint8 *)tbuf, UI_ALERT_INFO, UI_ALERT_ICON_INFO);
	if((len = tk_pop_by_tag(tags, len, STK_TAG_TEXT_STRING, tags)) == 255) return 0;
	
	len = tkGsm2Utf8(tags + 1, len, 128);
	status = if_ext_send(ectx, tags + 1, len);
	ui_alert_close(display, alert);
	if(status != 0) return STK_RES_LIMIT_SERVICE;
	return status;
} 

//prefetch proactive command
uint16 tk_decode_proactive_command(tk_context_p ctx, uint8 * command) {
	OS_DEBUG_ENTRY(tk_decode_proactive_command);
	uint8 tag;
	uint16 size;
	uint16 len = 0;
	uint8 blen;
	uint8 cmd_buf[8];
	uint8 localBuf[69];
	uint8 tbuf[70];
	uint16 wsize;
	uint16 i;
	//pop data tag
	tk_pop(command, &tag, &size, gba_apdu_data_field);
	wsize = size;
	//check for proactive command tag
	if(tag != FETCH_TAG_PROSIM) goto exit_decode_proactive_cmd;
	//pop command detail
	if((size = tk_pop_by_tag(gba_apdu_data_field, size, STK_TAG_CMD_DETAIL, cmd_buf)) == (WORD)-1) goto exit_decode_proactive_cmd;
	len = 0;
	//decode proactive command
	gb_cmd_qualifier = cmd_buf[2];		//command qualifier
	switch(cmd_buf[1]) {
		case STK_CMD_DISPLAY_TEXT:
			//tk_usb_callback(ctx, gba_apdu_data_field, wsize);
			tk_display_text(ctx, gb_cmd_qualifier, wsize, gba_apdu_data_field);
			gb_tk_state = TK_STATE_IDLE;
			if((blen = tk_pop_action(ctx, &tag, localBuf)) != 0) {
				if(tag != 0x21) break;
				len = tk_response(localBuf[0], 0, gba_apdu_data_field);
				goto dispatch_autoplay;
			}
			break;
		case STK_CMD_GET_INPUT:
			//tk_usb_callback(ctx, gba_apdu_data_field, wsize);
			tk_get_input(ctx, wsize, cmd_buf[2], gba_apdu_data_field);
			gb_tk_state = TK_STATE_IDLE;
			if((blen = tk_pop_action(ctx, &tag, localBuf)) != 0) {
				if(tag == 0x21) { 
					len = tk_response(localBuf[0], 0, gba_apdu_data_field);
				} else if(tag == 0x23) {
					len = tk_response(STK_RES_SUCCESS, 0, gba_apdu_data_field);
					len += tk_push(gba_apdu_data_field + len, STK_TAG_TEXT_STRING, len, localBuf);
				} else 
					break;
				goto dispatch_autoplay;
			}
			break;
		case STK_CMD_SELECT_ITEM:
			//tk_usb_callback(ctx, gba_apdu_data_field, wsize);
			tk_select_item(ctx, wsize, gba_apdu_data_field); 
			gb_tk_state = TK_STATE_IDLE;
			if((blen = tk_pop_action(ctx, &tag, localBuf)) != 0) {
				if(tag != 0x24) break;
				for(i =0; i<wsize; ) {
					i += tk_pop(gba_apdu_data_field + i, &tag, &size, tbuf);
					if((tag & 0x7F) == STK_TAG_ITEM) {
						tbuf[size] = 0;
						if(memcmp(tbuf + 1, localBuf, blen) == 0) {
							len = tk_response(STK_RES_SUCCESS, 0, gba_apdu_data_field);
							len += tk_push(gba_apdu_data_field + len, STK_TAG_ITEM_ID, 1, tbuf);
							goto dispatch_autoplay;
						}
					}
				}	
			}
			break;
		case STK_CMD_PROVIDE_LOCAL_INFORMATION:
			len = tk_local_info(ctx, gb_cmd_qualifier, wsize, gba_apdu_data_field);
			gb_tk_state = TK_STATE_LOCAL_INFO;
			break;
		case STK_CMD_SETUP_MENU:
			//set registered menu on memory
			memcpy(gba_registered_menu, gba_apdu_data_field, wsize);
			gb_registered_size = wsize;
			//tk_show_menu(ctx);
			break;
		dispatch_setup:
			len = tk_response(STK_RES_SUCCESS, 0, gba_apdu_data_field);
			if((len = tk_dispatch(ctx, len, gba_apdu_data_field)) == 0) {
				gb_tk_state = TK_STATE_IDLE;
			} else {
				gb_tk_state = TK_STATE_EXECUTE;
			}
			break;
		case STK_CMD_SETUP_EVENT_LIST:
			break;
			goto dispatch_setup;
		case STK_CMD_TIMER_MANAGEMENT:
			
			break;
		//extended proactive commands
		case STK_CMD_SEND_REQ_EXT:
		case STK_CMD_SEND_REQ:
			len = wsize;
			gb_tk_state = TK_STATE_SEND_REQ;
			break;
		/* slave device IO management */
		case STK_CMD_SEND_DEVICE_EXT:
		case STK_CMD_SEND_DEVICE:
			len = wsize;
			gb_tk_state = TK_STATE_SEND_DEVICE;
			break;
		/* payment terminal machine */
		case STK_CMD_SEND_PRINTER:
			len = wsize;
			gb_tk_state = TK_STATE_SEND_PRINTER;
			break;
		case STK_CMD_INVOKE_KERNEL:
			len = wsize;
			gb_tk_state = TK_STATE_INVOKE_KERNEL;
			break;
		case STK_CMD_LOAD_RESOURCE:
			len = wsize;
			gb_tk_state = TK_STATE_LOAD_RESOURCE;
			break;
		case STK_CMD_STORE_RESOURCE:
			len = wsize;
			gb_tk_state = TK_STATE_STORE_RESOURCE;
			break;
		default:
			gb_tk_state = TK_STATE_EXECUTE;
			break;
	}
	goto exit_decode_proactive_cmd;
	dispatch_autoplay:
	//dispatch pre-constructed command response
	if((len = tk_dispatch(ctx, len, gba_apdu_data_field)) == 0) {
		gb_tk_state = TK_STATE_IDLE;
		ctx->runstate &= TK_AUTOPLAY_ENABLED;		//disable autorun/autorec
		ctx->offset = SHARD_AUTOREC_START;
	} else {
		gb_tk_state = TK_STATE_EXECUTE;
	}
	exit_decode_proactive_cmd:
	OS_DEBUG_EXIT();
	return len;
}
void tk_init(tk_context_p ctx, uint8 * command) {
	OS_DEBUG_ENTRY(tk_init);
	uint16 len;
	BYTE status;
	ui_object * obj;
	while(1) {
		status = STK_RES_SUCCESS;
		switch(gb_tk_state) {

			case TK_STATE_LIST_MENU:		//menu registration
				if((len = tk_setup_menu(ctx)) == 0) goto exit_tk_init;
				gb_tk_state = TK_STATE_EXECUTE;
			case TK_STATE_EXECUTE:			//startup event
			case TK_STATE_LIST_EVENT:		//event registration
				len = tk_decode_proactive_command(ctx, command);
				
				break;

			case TK_STATE_IDLE:
				len = 0;
				//display registered menu, wait for user input
				tk_show_menu(ctx);
				goto exit_tk_init;
		}
		//construct terminal response
		len = tk_response(status, 0, gba_apdu_data_field);
		if((len = tk_dispatch(ctx, len, gba_apdu_data_field)) == 0) {
			gb_tk_state = TK_STATE_IDLE;
		} else {
			switch(gb_tk_state) {

				case TK_STATE_LIST_MENU:
					gb_tk_state = TK_STATE_LIST_EVENT;
					break;
				default:
				case TK_STATE_LIST_EVENT:
					gb_tk_state = TK_STATE_EXECUTE;
					break;
			}
		}
	}
	exit_tk_init:
	OS_DEBUG_EXIT();
	return;
}

void tk_decode(tk_context_p ctx, uint8 * command) {
	OS_DEBUG_ENTRY(tk_decode);
	uint16 len;
	uint16 rlen;
	uint8 status;
	ui_object * obj;
	ui_alert_p alert;
	ext_context xctx;
	uint8 temp_buffer[256];
	//if_ext_init(&xctx);
	//if_ext_sendstring(&xctx, (uint8 *)"Card Inserted\r\n");
	alert = ui_alert_show(ctx->display, (uint8 *)"Loading", (uint8 *)"Please Wait...", UI_ALERT_INFO, UI_ALERT_ICON_BUSY);
	tk_init(ctx, command);
	if(alert != NULL) ui_alert_close(ctx->display, alert);
	while(1) {
		status = STK_RES_SUCCESS;
		switch(gb_tk_state) {

			case TK_STATE_EXECUTE:
				rlen = tk_decode_proactive_command(ctx, command);
				break;
			case TK_STATE_IDLE:
				len = 0;
				//display registered menu, wait for user input
				tk_show_menu(ctx);
				//check for autoplay menu
				rlen = tk_autoplay_menu(ctx);
				break;
		}
		//start ui rendering presentation (enter loop)
#if SHARD_RTOS_ENABLED
		while(gb_tk_state == TK_STATE_IDLE) {
			obj = tk_wait_user_input(ctx);
		}
#else
		while(gb_tk_state == TK_STATE_IDLE) {
			obj = tk_present(ctx);
			if(if_card_state(ctx->cctx) != 0)  gb_tk_state = TK_STATE_CARD_DISCONNECTED;
		}
#endif
		//if(gb_tk_state == TK_STATE_LIST_DELETE) goto tk_exec_cleanup;
		if(gb_tk_state == TK_STATE_CARD_DISCONNECTED) {
			gb_tk_state = TK_STATE_LIST_APP;
			goto tk_exec_cleanup;
		}
		//already dispatched
		if(gb_tk_state == TK_STATE_EXECUTE) { len = rlen; continue; }
		//clear response (status only)
		len = 0;		
		//start input processing
		switch(gb_tk_state) {
			case TK_STATE_USB_COMMAND:
				//USB command processing , add new command on this state machine
				switch(ctx->ucmd) {
					case TK_UCMD_RESULT:
						len = tk_response(ctx->ubuf[0], 0, gba_apdu_data_field);
						break;
					case TK_UCMD_ITEM:
						len = tk_response(0x00, 0, gba_apdu_data_field);
						len += tk_push(gba_apdu_data_field + len, STK_TAG_ITEM_ID, 1, ctx->ubuf);
						break;
					case TK_UCMD_TEXT:
						tk_memcpy(temp_buffer + 1, ctx->ubuf, ctx->ulen);
						rlen = ctx->ulen + 1;
						//should set DCS according to user input
						temp_buffer[0] = 0x04;
						//construct terminal response
						len = tk_response(0, 0, gba_apdu_data_field);
						len += tk_push(gba_apdu_data_field + len, STK_TAG_TEXT_STRING, rlen, temp_buffer);
						break;
				}
				break;
			case TK_STATE_ENVELOPE_MENU:
				//construct terminal response
				len = tk_response(status, 0, gba_apdu_data_field);
				len += tk_push(gba_apdu_data_field + len, STK_TAG_ITEM_ID, 1, &((ui_item *)obj)->id);
				//use envelope menu
				len = tk_push(gba_apdu_data_field, ENV_TAG_MENU, len, gba_apdu_data_field);
				tk_push_action(ctx, 0x25, (uint8 *)((ui_item *)obj)->base.text, strlen(((ui_item *)obj)->base.text));
				break;
			case TK_STATE_DISPLAY_TEXT:
				//wait for user input
				status = ((ui_button *)obj)->id;
				//construct terminal response
				len = tk_response(status, 0, gba_apdu_data_field);
				tk_push_action(ctx, 0x21, &status, 1);
				break;

			case TK_STATE_SELECT_ITEM:
				//wait for user input
				len = tk_response(0x00, 0, gba_apdu_data_field);
				len += tk_push(gba_apdu_data_field + len, STK_TAG_ITEM_ID, 1, &((ui_item *)obj)->id);
				tk_push_action(ctx, 0x24, (uint8 *)((ui_item *)obj)->base.text, strlen(((ui_item *)obj)->base.text));
				break;

			case TK_STATE_GET_INPUT:
				//wait for user input
				status = ((ui_button *)obj)->id;
				rlen = strlen((const char *)((ui_textbox *)obj->target)->content);
				tk_memcpy(temp_buffer + 1, ((ui_textbox *)obj->target)->content, rlen);
				rlen += 1;
				//should set DCS according to user input
				temp_buffer[0] = 0x04;
				//construct terminal response
				len = tk_response(status, 0, gba_apdu_data_field);
				if(status != STK_RES_TERMINATED) { 
					len += tk_push(gba_apdu_data_field + len, STK_TAG_TEXT_STRING, rlen, temp_buffer);
					tk_push_action(ctx, 0x23, temp_buffer, rlen);		//USB action
				} else {
					tk_push_action(ctx, 0x21, &status, 1);
				}
				break;
			case TK_STATE_GET_DATETIME:			//special input
				status = ((ui_datetime_item *)obj)->result;
				rlen = strlen((const char *)((ui_datetime_item *)(obj->target))->dtval);
				tk_memcpy(((ui_datetime_item *)(obj->target))->dtval + 1, ((ui_datetime_item *)(obj->target))->dtval, rlen);
				rlen += 1;
				//should set DCS according to user input
				((ui_datetime_item *)obj->target)->dtval[0] = 0x04;
				//construct terminal response
				len = tk_response(status, 0, gba_apdu_data_field);
				if(status != STK_RES_TERMINATED) { 
					len += tk_push(gba_apdu_data_field + len, STK_TAG_TEXT_STRING, rlen, ((ui_datetime_item *)obj->target)->dtval);
					tk_push_action(ctx, 0x23, ((ui_textbox *)obj->target)->content, rlen);
				} else {
					tk_push_action(ctx, 0x21, &status, 1);
				}
				break;
			case TK_STATE_SEND_DEVICE:			//send extension port
				rlen = tk_send_device(ctx, gb_cmd_qualifier & 0x7F, rlen, gba_apdu_data_field);
				
				//construct terminal response
				if(rlen == 0) {
					status = STK_RES_TRANSACTION_ABORT;
					len = tk_response(status, 0, gba_apdu_data_field);
				} else {
					//rlen = tkUtf82Gsm(gba_net_buffer, rlen, 255);
					len = tk_response(status, 0, gba_apdu_data_field);
					len += tk_push(gba_apdu_data_field + len, STK_TAG_CHANNEL_DATA, rlen, gba_net_buffer);
				}
				break;
			case TK_STATE_INVOKE_KERNEL:
				rlen = tk_kernel_invoke(ctx, gb_cmd_qualifier & 0x7F, rlen, gba_apdu_data_field);
				memcpy(temp_buffer, gba_apdu_data_field, rlen);			//use temp_buffer to store the result
				//construct terminal response
				status = STK_RES_SUCCESS;
				len = tk_response(status, 0, gba_apdu_data_field);
				switch(gb_cmd_qualifier & 0x07) {
					case 0:		//invoke by alias
						if(rlen != 0) len += tk_push(gba_apdu_data_field + len, STK_TAG_TEXT_STRING, rlen, temp_buffer);
						break;
					case 1:		//load instance
						if(rlen != 0) len += tk_push(gba_apdu_data_field + len, STK_TAG_INSTANCE, rlen, temp_buffer);
						break;
					case 2:		//invoke by method
						if(rlen == (uint16)-1) {
							temp_buffer[0] = VX_UNRESOLVED_METHOD;
							len += tk_push(gba_apdu_data_field + len, STK_TAG_ERROR, 1, temp_buffer);
						} else if(rlen < 256) {
							if(rlen != 0) len += tk_push(gba_apdu_data_field + len, STK_TAG_RETURN, rlen, temp_buffer);
						}
						break;
					default: break;
				}
				break;
			case TK_STATE_LOAD_RESOURCE:
				rlen = tk_load_resource(ctx, gb_cmd_qualifier & 0x7F, rlen, gba_apdu_data_field);
				memcpy(temp_buffer, gba_apdu_data_field, rlen);			//use temp_buffer to store the result
				status = STK_RES_SUCCESS;
				len = tk_response(status, 0, gba_apdu_data_field);
				if(rlen != 0) len += tk_push(gba_apdu_data_field + len, STK_TAG_RESOURCE_ID, rlen, temp_buffer);
				break;
			case TK_STATE_STORE_RESOURCE:
				status = tk_store_resource(ctx, gb_cmd_qualifier & 0x7F, rlen, gba_apdu_data_field);
				len = tk_response(status, 0, gba_apdu_data_field);
				break;
			case TK_STATE_SEND_REQ:
				//choose command qualifier
				
				rlen = tk_send_request(ctx, gb_cmd_qualifier & 0x7F, rlen, gba_apdu_data_field);
				
				//construct terminal response
				if(rlen == 0) {
					status = STK_RES_TRANSACTION_ABORT;
					len = tk_response(status, 0, gba_apdu_data_field);
				} else {
					//rlen = tkUtf82Gsm(gba_net_buffer, rlen, 255);
					len = tk_response(status, 0, gba_apdu_data_field);
					len += tk_push(gba_apdu_data_field + len, STK_TAG_CHANNEL_DATA, rlen, gba_net_buffer);
				}
				break;
			case TK_STATE_SEND_PRINTER:
				status = tk_printer_send(ctx->display, &xctx, rlen, gba_apdu_data_field);
				len = tk_response(status, 0, gba_apdu_data_field);
				break;
			case TK_STATE_LOCAL_INFO:
				if(rlen == 0) {
					status = STK_RES_SUCCESS;
					len = tk_response(status, 0, gba_apdu_data_field);
				} else {
					memcpy(temp_buffer, gba_apdu_data_field, rlen);
					len = tk_response(status, 0, gba_apdu_data_field);
					switch(gb_cmd_qualifier) {
						case 1: 		//ESID (terminal id)
							len += tk_push(gba_apdu_data_field + len, STK_TAG_IMEI, rlen, temp_buffer);
							break;
						case 3: 		//datetime (local time)
							len += tk_push(gba_apdu_data_field + len, STK_TAG_DATETIMEZONE, rlen, temp_buffer);
							break;
						default: 
							break;
					}
				}
				break;
			default: 
				//terminate toolkit session
				status = STK_RES_TERMINATED;
				len = tk_response(status, 0, gba_apdu_data_field);
				len = tk_dispatch(ctx, len, gba_apdu_data_field);
				goto tk_exec_cleanup;		//unresolved state (cleanup)
		}
		//dispatch pre-constructed command response
		if((len = tk_dispatch(ctx, len, gba_apdu_data_field)) == 0) {
			gb_tk_state = TK_STATE_IDLE;
			ctx->runstate &= TK_AUTOPLAY_ENABLED;		//disable autorun/autorec
			ctx->offset = SHARD_AUTOREC_START;
		} else {
			gb_tk_state = TK_STATE_EXECUTE;
		}
	}
	tk_exec_cleanup:
	ctx->app_state = TK_APP_STATE_DORMANT;
	ctx->runstate &= TK_AUTOPLAY_ENABLED;		//disable autorun/autorec
	ctx->offset = SHARD_AUTOREC_START;
	//orblet session clean-up
	//if_http_clear_header(ctx->netctx);	//clear HTTP header from current session
	OS_DEBUG_EXIT();
}

static uint8 tk_card_authenticate(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_card_authenticate);
	uint16 rlen;
	uint16 status;
	uint16 lrc;
	uint8 challenge[16];
	uint8 keybuf[16];
	uint8 response[18];
	uint8 ret = -1;
	cr_context crypto;
	//GET CHALLENGE
	status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu(gba_apdu_buffer, 0x00, 0x84, 0x00, 0x00, 0x10), gba_apdu_data_field, &rlen);
	if(status == 0x6E00) goto exit_card_auth;
	if(status != 0x9000) goto exit_card_auth;
	tk_memcpy(challenge, gba_apdu_data_field, 16);
	//challenge accepted (total 16 bytes)
	cr_init_pandora();
	//generate key (pandora key exchange)
	cr_generate_key((char *)gba_apdu_data_field, (char *)keybuf, 0x10);
	//generate pandora key (shared with card)
	cr_randomize(ctx->pkey, 0x10);
	tk_memcpy(response + 2, ctx->pkey, 0x10);		//copy generated key to response
	//encrypt response with keybuf
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_ECB | CR_MODE_ENCRYPT, response + 2); 
	crypto.key = keybuf;
	cr_do_crypt(&crypto, 0, 16);
	//finalize pandora
	lrc = cr_finalize_key((char *)keybuf, 0x10, 0);
	//clear keybuf (use only once during authentication)
	memset(keybuf, 0, 0x10);
	//external authenticate
	response[0] = lrc >> 8;
	response[1] = lrc & 0xFF;
	status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu_data(gba_apdu_buffer, 0x00, 0x82, 0x00, 0x00, 0x12, response), gba_apdu_data_field, &rlen);
	if(status != 0x9000) goto exit_card_auth;
	if(rlen != 0x10) goto exit_card_auth;		//card cryptogram failed
	//try authenticate card using new pandora key
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_ECB | CR_MODE_DECRYPT, gba_apdu_data_field); 
	crypto.key = ctx->pkey;
	cr_do_crypt(&crypto, 0, 16);
	if(memcmp(gba_apdu_data_field, challenge, 16) != 0) ret = -1;
	else ret = 0;
	exit_card_auth:
	OS_DEBUG_EXIT();
	return ret;
}

static void tk_activate_card(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_activate_card);
	json_t * json;
    json_error_t error;
	int rv;
	uint8 url_buffer[256];
	uint8 div_buffer[24];
	uint8 esid_buffer[33];
	ssl_cert_p cert = NULL;
	net_request req;
	uint16 rlen;
	uint16 status;
	uint16 len;
	int result;
	uint8 lcs = 0x07;		//activated state
	if(ctx->flag & TK_FLAG_CARD_SYNCHED) goto exit_card_activate;
	if(ctx->flag & TK_FLAG_CARD_DORMANT) {
		status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu(gba_apdu_buffer, 0x80, 0xCA, 0x00, 0xDD, 0x00), gba_apdu_data_field, &rlen);
		if(status == 0x9000) {
			tk_bin2hex(gba_apdu_data_field + 2, 10, div_buffer);
			tk_bin2hex(ctx->esid, SHARD_ESID_SIZE, esid_buffer);
			sprintf((char *)url_buffer, "http://orbleaf.com/apis/river/act.php?div=%s&esid=%s", div_buffer, esid_buffer);
			//try sending HTTP request for card activation
			net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert);
			len = http_send(ctx->netctx, &req, NULL, NULL, 0, gba_net_buffer);
			if(len != 0) {
				gba_net_buffer[len] = 0;
				json = json_loads((const char *)gba_net_buffer, JSON_DECODE_ANY, &error);
				if(json == NULL) goto set_flag_synched;
				rv = json_unpack(json, "{s:s, s?i}", "auth", gba_apdu_data_field, "status", &result);
				if(rv != 0) goto set_flag_synched;
				if(result == 0) lcs = 0x07;													//enable card
				else lcs = 0x0C;																//disable card
				len = tk_hex2bin(gba_apdu_data_field, gba_apdu_data_field);		//send authcode from server directly to card w/o any modification
				//change current card lifecycle state
				status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu_data(gba_apdu_buffer, 0x80, 0xFE, 0x00, lcs, len, gba_apdu_data_field), gba_apdu_data_field, &rlen);
				json_object_clear(json);
			}
		}
		set_flag_synched:
		ctx->flag |= TK_FLAG_CARD_SYNCHED;		//request for activation already sent
	}
	exit_card_activate:
	OS_DEBUG_EXIT();
}

void tk_main_loop(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_main_loop);
	ui_object * obj;
	uint8 err;
	ui_alert * alert;
	ui_object * btn;
	//authenticate card first
	tk_card_authenticate(ctx);
	//initialize owl host
	//owl_init(ctx->octx);
	while(1) {
		switch(gb_tk_state) {
			case TK_STATE_LIST_APP:
				if(if_card_state(ctx->cctx) != 0) goto card_disconnected;
				if((err = tk_list_application(ctx)) != (uint8)-1) break;			//app already selected		
				gb_tk_state = TK_STATE_IDLE;
				if(gb_tk_state == TK_STATE_IDLE) {
#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON
					if(ctx->ucmd == TK_UCMD_SELECT_APP) {
						ctx->ucmd = TK_UCMD_NONE;
						if((err = tk_select_application(ctx, ctx->ulen, ctx->ubuf)) != 0) {
							gb_tk_state = TK_STATE_CONFIG_ERROR; //display error before back to list_app
						} else {
							gb_tk_state = TK_STATE_LIST_MENU; //back to list app
							goto execute_cat;
						}
						break;
					}
#endif
					//activate card immediately as soon as application listed
					tk_activate_card(ctx);	
					//start ui rendering presentation (enter loop)
#if SHARD_RTOS_ENABLED
					while(gb_tk_state == TK_STATE_IDLE) {
						obj = tk_wait_user_input(ctx);
					}
#else
					while(gb_tk_state == TK_STATE_IDLE) {
						obj = tk_present(ctx);
						if(if_card_state(ctx->cctx) != 0)  gb_tk_state = TK_STATE_CARD_DISCONNECTED;
					}	
#endif
				} 
				break;
			case TK_STATE_RIVER_USB_DOWNLOAD:
				if((err = tk_select_application(ctx, ctx->cmlen, ctx->cmaid)) != 0) {
					gb_tk_state = TK_STATE_CONFIG_ERROR;
					break;
				}
				goto start_river;
			case TK_STATE_RIVER:
				if((err = tk_select_application(ctx, ((ui_app_item *)obj)->aidlen, ((ui_app_item *)obj)->aid)) != 0) {
					gb_tk_state = TK_STATE_CONFIG_ERROR;
					break;
				}
				start_river:
				tk_river_loop(ctx);
				gb_tk_state = TK_STATE_LIST_APP;
				break;
			case TK_STATE_CARD_DISCONNECTED: 
				card_disconnected:
				tk_clear_body(ctx);
				goto exit_main_loop;
				break;
			
			case TK_STATE_LIST_DELETE:			//list application to be deleted
				if(if_card_state(ctx->cctx) != 0)  { gb_tk_state = TK_STATE_CARD_DISCONNECTED; break; }
				if(tk_list_app_for_delete(ctx) != 0) { gb_tk_state = TK_STATE_CARD_DISCONNECTED; break; }
				gb_tk_state = TK_STATE_IDLE;
				if(gb_tk_state == TK_STATE_IDLE) {
					//start ui rendering presentation (enter loop)		
#if SHARD_RTOS_ENABLED
					while(gb_tk_state == TK_STATE_IDLE) {
						obj = tk_wait_user_input(ctx);
					}
#else
					while(gb_tk_state == TK_STATE_IDLE) {
						obj = tk_present(ctx);
						if(if_card_state(ctx->cctx) != 0)  gb_tk_state = TK_STATE_CARD_DISCONNECTED;
					}
#endif
				} 
				break;
			case TK_STATE_USB_SELECT:			//select app from USB
				//card is inserted
				if(tk_select_application(ctx, ctx->ulen, ctx->ubuf) != 0) {
					gb_tk_state = TK_STATE_CONFIG_ERROR; //display error before back to list_app
				} else {
					gb_tk_state = TK_STATE_LIST_MENU; //back to list app
					//goto execute_cat;
				}	
				break;
			case TK_STATE_DELETE_APP:
				tk_confirm_delete(ctx, ctx->aidlen, ctx->aidbuf);
				break;
			case TK_STATE_USB_DELETE_APP:
				tk_delete_application(ctx, ctx->aidlen, ctx->aidbuf); 
				ctx->aidlen = 0;
				memset(ctx->aidbuf, 0, TK_MAX_AIDLEN);
				gb_tk_state = TK_STATE_LIST_APP;
				break;
			case TK_STATE_CONFIG_ERROR:
				if(err == (uint8)-1) { gb_tk_state =  TK_STATE_CARD_DISCONNECTED; break; }
				
				alert = ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)g_strErrConfText[err], UI_ALERT_BUTTON_OK, UI_ALERT_ICON_ERROR);
				if(alert != NULL) {
					gb_tk_state = TK_STATE_IDLE;
					//start ui rendering presentation (enter loop)	
#if SHARD_RTOS_ENABLED
					btn = tk_wait_user_input(ctx);
#else
					while(gb_tk_state == TK_STATE_IDLE) {
						btn = tk_present(ctx);
						if(if_card_state(ctx->cctx) != 0)  gb_tk_state = TK_STATE_CARD_DISCONNECTED;
					}
#endif
					if(alert != NULL) ui_alert_close(ctx->display, alert);
				}
				gb_tk_state = TK_STATE_LIST_APP; 		//back to list app
				break;
			default:										//default select application
				if((err = tk_select_application(ctx, ((ui_icon *)obj)->aidlen, ((ui_icon *)obj)->aid)) != 0) { 
					gb_tk_state = TK_STATE_CONFIG_ERROR; //back to list app
					break;		//selection failed (configuration problem maybe)
				} else {
					gb_tk_state = TK_STATE_LIST_MENU;
				}
			case TK_STATE_LIST_MENU:
			case TK_STATE_EXECUTE:
				execute_cat:
				//execute CAT
				tk_decode(ctx, gba_apdu_data_field) ;
				//if(gb_tk_state == TK_STATE_CARD_DISCONNECTED) {
				//	gb_tk_state = TK_STATE_LIST_APP; //back to list app
				//}
				break;
		}
	}
	exit_main_loop:
	//deinitialize owl host (stopping clock generation)
	//owl_deinit(ctx->octx);
	OS_DEBUG_EXIT();
	return;
}

void tk_task_power() {
	uint8 err;
	uint8 sleep = FALSE;
	uint32 csr;
	uint32 flag;
	tk_context_p ctx = (tk_context_p)os_get_context();
	while(1) {
		if(ctx->cos_status & TK_COS_STAT_VM_STARTED) g_counter = 0;		//reset counter if vm is running
		if(NET_BUSY(ctx->netctx)) g_counter = 0;		//reset counter if net is busy
		if(g_counter > 600 && (ctx->cos_status & TK_COS_STAT_PICC_AVAIL) == 0) {
			//enter power down mode (disable net, card and timer)
			err = 0;
			//turnoff lcd backlight
			if_display_sleep(ctx->display);
			((gui_handle_p)ctx->display)->set_backlight(ctx->display, 0);
			//disable task scheduling, enter sleep mode
			sleep_again:
			err |= if_ble_sleep(ctx->bctx);
			err |= if_net_power_down(ctx->netctx);
			err |= if_nfc_sleep(ctx->rctx);
			err |= if_card_sleep(ctx->cctx);
			err |= if_timer_sleep();
			if(err == 0) if_sys_sleep();
			csr = os_enter_critical();
			//HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
			if(err == 0) HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);	//stop clock
			else HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);		//still need clock
			__nop();
			__nop();
			__nop();
			__nop();
			__nop();
			__nop();
			__nop();
			os_exit_critical(csr);
			if(err == 0) if_sys_wake();
			if_timer_wake();
			if_card_wake(ctx->cctx);	
			if_nfc_begin(ctx->rctx);					//wake nfc device
			if_net_power_up(ctx->netctx);
			if_ble_wake(ctx->bctx);
			flag = if_pwr_get_interrupt_source();
			while(1) {
				if_pwr_clear_interrupt_source();
				if(flag == 0) goto sleep_again;			//no interrupt avail
				if((flag & IF_INT_WAKE) != 0) break;		//wake type interrupt detected, exit sleep
				os_wait(2000);
				while(ctx->cos_status & TK_COS_STAT_VM_STARTED) os_wait(200);
				flag = if_pwr_get_interrupt_source();
			}
			//exit sleep mode (enable card, timer)
			if_display_wake(ctx->display);
			os_wait(80);
			os_wait(100);
			((gui_handle_p)ctx->display)->set_backlight(ctx->display, 1);		//display on
			g_counter = 0;
		}
		//check for battery, auto shutdown when battery is lower than 4%
	//#if SHARD_AUTO_POWER_DOWN
		while(if_pwr_get_batt_percent() < 8) {
			err = 0;
			err |= if_ble_sleep(ctx->bctx);
			err |= if_net_power_down(ctx->netctx);
			err |= if_nfc_sleep(ctx->rctx);
			err |= if_card_sleep(ctx->cctx);
			err |= if_timer_sleep();
			if_sys_sleep();
			((gui_handle_p)ctx->display)->set_backlight(ctx->display, 0);		//shutdown display
			csr = os_enter_critical();
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
			__nop();
			__nop();
			__nop();
			__nop();
			__nop();
			__nop();
			__nop();
			os_exit_critical(csr);
			if_sys_wake();
			sleep = TRUE;	
		}
		//released from sleep
		if(sleep) {
			if_net_power_up(ctx->netctx);
			if_timer_wake();
			if_card_wake(ctx->cctx);	
			if_nfc_begin(ctx->rctx);					//wake nfc device
			if_display_wake(ctx->display);
			if_ble_wake(ctx->bctx);
			((gui_handle_p)ctx->display)->set_backlight(ctx->display, 1);		//display on
		}
		os_wait(1000);
	}
}

void tk_task_app() {
	tk_context_p ctx = os_get_context();
	//while(1) {
		//os_kill_task(os_get_active_task());
		//os_suspend();
		//tk_console_print(ctx, "running framework");	
	
		//vm_decode();
	exit_invoke_kernel:
		//check if application is currently running, terminate application
		//vm_close();
		//ctx->cos_status &= ~(TK_COS_STAT_VM_STARTED | TK_COS_STAT_PICC_AVAIL);
		//os_resume(os_find_task_by_name("worker"));
	//}
	while(1);
}

static void tk_net_exception_handler(net_context_p ctx, const char * text) {
	tk_console_print(ctx, (char *)text);
}

static void tk_net_prepare_handler(void * ctx) {
	while(NFC_BUSY(((tk_context_p)ctx)->rctx) != 0) { os_wait(100); }		//wait until nfc is not busy
	if(if_nfc_open(((tk_context_p)ctx)->rctx) == 0) {
		if_nfc_set_rf_field(((tk_context_p)ctx)->rctx, PN532_RF_DISABLE);
	}
}

static void tk_net_finish_handler(void * ctx) {
	if_nfc_set_rf_field(((tk_context_p)ctx)->rctx, PN532_RF_ENABLE);
	if_nfc_close(((tk_context_p)ctx)->rctx);
}

void tk_task_worker() {
	OS_DEBUG_ENTRY(tk_task_worker);
	ui_signal * signal;
	uint8 i;
	uint32 csr;
	uint8 toggle_state = 0;
	uint8 state = 0;
	ui_object * setting;
	tk_context_p ctx = (tk_context_p)os_get_context();
	if_delay(2000);
	if_net_wake(ctx->netctx);
	if_net_try_connect(ctx->netctx);
	os_create_task(ctx, tk_task_power, "pwr", 0xfff0, 1024);		//lowest priority task
	//os_create_task(ctx, tk_task_app, "app", 0xeff0, 2048 + VA_OBJECT_MAX_SIZE);
	for(state = 0;state<7; state++) {
		//check for user activity within 24 seconds, power saving mode
		if(ctx->netctx->state & IF_NET_STATE_CONNECTED && (ctx->cos_status & TK_COS_STAT_PICC_AVAIL) == 0) {
			switch(state & 0x07) {
				case 1:
					tk_console_print(ctx, "synching framework");
					if(tk_kernel_devlet_sync(ctx) == 0) break; //sync app framework
				case 2:
					tk_console_print(ctx, "checking update");
					if(tk_check_update(ctx) == 0) break;			//check for firmware update
				case 0: 
					tk_console_print(ctx, "synching time");
					if(tk_sync_time(ctx) == 0) break;				//sync time with ntp server
				case 3:
					if(if_ble_try_connect(ctx->bctx) == 0) break;
				default: 
					break;
			}
		}
	}
	if_net_sleep(ctx->netctx);
	if(ctx->rctx->nfc_state & NFC_STATE_INITIALIZED) tk_console_print(ctx, "NFC ready");
	else tk_console_print(ctx, "NFC unavailable");
	//initialize kernel application (in-case exist)
	tk_kernel_init(ctx);
	while(1) {
		uint8 percent = if_pwr_get_batt_percent();
		signal = (ui_signal *)ui_get_object_by_name(ctx->display, "xsignal") ;
		if(signal != NULL) {
			signal->batt = percent;
			ui_reinitialize_object((ui_object *)signal);
		}
		for(i=0;i<15;i++) {
			if(!NET_BUSY(ctx->netctx)) {
#if (SHARD_NFC_TYPE == 0x522)
				if((ctx->cos_status & TK_COS_STAT_PICC_AVAIL) == 0) {
					if((atrlen = if_picc_connect(ctx->rctx, atr)) != (uint8)-1) {
						//picc card connected, invoke event 81h (if exist)
						if(ctx->rctx->uid.sak & PICC_TYPE_ISO_14443_4) {
							//default ISO14443-4 compliant card (future Orb-Weaver)
						} else if(ctx->rctx->uid.sak & PICC_TYPE_ISO_18092) {
							//default ISO18092 compliant card (NFC - NTAG)
						}else {
							//mifare only (RC522 only support mifare card)
							switch(ctx->rctx->uid.sak & 0x1F) {
								case PICC_TYPE_MIFARE_MINI	:			//0x09//;
								case PICC_TYPE_MIFARE_1K:				//0x08//;
								case PICC_TYPE_MIFARE_4K:				//0x18//;
								case PICC_TYPE_MIFARE_UL:				//0x00//;
								case PICC_TYPE_MIFARE_PLUS:			//0x10	//;
									if(tk_kernel_trigger_event(ctx, VM_EVENT_PICC_IN) == 0) {
										ctx->cos_status |=  TK_COS_STAT_PICC_AVAIL;
									}
									break;
							}
						}
						g_counter = 0;			//reset counter
					}
				}
#endif
#if (SHARD_NFC_TYPE == 0x532)
				if((ctx->cos_status & TK_COS_STAT_PICC_AVAIL) == 0) {
					//try mifare (without initiator data)
					if(if_nfc_list_passive_target(ctx->rctx, 2, PN532_MIFARE_ISO14443A, NULL, 0) == 0) {
						if(tk_kernel_trigger_event(ctx, VM_EVENT_PICC_IN, ctx->rctx->uidLen, ctx->rctx->uid) == 0) {
							ctx->cos_status |=  TK_COS_STAT_PICC_AVAIL;
							g_counter = 0;			//reset counter
						}
					}
				}
#endif
				//check when vm is started, execute vm_decode
				if(ctx->cos_status & TK_COS_STAT_VM_STARTED) {	
					//os_suspend();
					//clear any resources from previous session
					//os_resume(os_find_task_by_name("app"));
					//os_suspend();
					//tk_clear_resources(ctx);
#if (SHARD_NFC_TYPE == 0x522)
					//if_rfid_init(ctx->rctx);							//rfid re-init		(2017.06.29)
#endif
#if (SHARD_NFC_TYPE == 0x532)
					//if_nfc_init_picc(ctx->rctx);
#endif
					//tk_clear_body(ctx);
				}
			}
			if((i % 4) == 0) {
				if(ctx->rctx->nfc_state & NFC_STATE_ENABLED) {
					setting = ui_get_object_by_name(ctx->display, "mySetting");
					if(setting != NULL) {
						toggle_state = !toggle_state;
						ui_set_toggle_state(setting, toggle_state);
					}
				}
			}
			if((i % 10) == 0) {
				//check for ble connection
				if_ble_try_connect(ctx->bctx) ;
			}
			tk_net_prepare_handler(ctx);
			os_wait(950);		//15 seconds
			tk_net_finish_handler(ctx);
			os_wait(50);		//15 seconds
			//check for socket listener event
			if_net_exec_listener(ctx->netctx);
		}
		if(if_gps_get(ctx->gpsctx) == 0) {
			tk_console_print(ctx, "location detected");
		}
		setting = ui_get_object_by_name(ctx->display, "mySetting");
		if(setting != NULL) {
			toggle_state = 0;
			ui_set_toggle_state(setting, toggle_state);
		}
		//check connection
		if_net_wake(ctx->netctx);
		if_net_try_connect(ctx->netctx);
		if_net_sleep(ctx->netctx);
	}
//#endif
	OS_DEBUG_EXIT();
}


static ui_object * g_triggered_obj;
static uint16 g_frame_counter = 0;
void tk_task_status() {
	OS_DEBUG_ENTRY(tk_task_status);
	tk_context_p ctx;
	ctx = os_get_context();
	ui_infostat * infostat;
	while(1) {
		g_frame_counter = 0;
		os_wait(1000);
		infostat = (ui_infostat *)ui_get_object_by_name(ctx->display, "infoBar");
		//infostat = (ui_infostat *)((gui_handle_p)ctx->display)->meta;
		if(infostat != NULL) {
			infostat->fps = (g_frame_counter + infostat->fps) / 2;
		}
	}
	
	OS_DEBUG_EXIT();
	
}
void tk_task_render() {
	OS_DEBUG_ENTRY(tk_task_render);
	tk_context_p ctx;
	ui_object * obj;
	ui_object * wobj;
	os_message * msg;
	uint8 orientation = 0;
	uint8 i;
	ctx = os_get_context();
	uint16 tick = 1000 / ((gui_handle_p)ctx->display)->fps;
	while(1) {
		msg = os_dequeue_message();
		wobj = ui_get_object_by_name(ctx->display, "infoBar");
		if(msg != NULL && msg->reqlen != 0) {
			ui_set_text((ui_text *)wobj, msg->request);
			ui_reinitialize_object(wobj);
			os_delete_message(msg);
		} else {
			ui_set_text((ui_text *)wobj, (uint8 *)"");
		}
		//check for screen orientation
		if(orientation != ctx->config->orientation) {
			orientation = ctx->config->orientation;
			//rotate orientation
			ui_switch_orientation(ctx->display, orientation);
			//reinitialize all objects
			ui_reinitialize_object(((gui_handle_p)ctx->display)->header);
			ui_reinitialize_object(((gui_handle_p)ctx->display)->body);
			ui_reinitialize_object(((gui_handle_p)ctx->display)->meta);
		}
		
		for(i=0;i<30;i++) {
			//if((i % 10) == 0) {
			//	ui_reinitialize_object((ui_object *)wobj);
			//}
			obj = tk_present(ctx);
			if(obj != NULL) {
				g_triggered_obj = obj;
			}
			g_frame_counter++;
			os_wait(tick);		//14-15fps
			//os_wait(200);		//14-15fps
		}
	}
	OS_DEBUG_EXIT();
}

void tk_console_print(void * ctx, char * text) {
	uint16 ret;
	os_message * msg;
	//message should already deleted in-case response buffer == NULL
	msg = os_send_message(os_find_task_by_name("render"), os_create_message(ctx, text, strlen(text), NULL));
	//os_delete_message(msg);
}

ui_object * tk_wait_user_input(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_wait_user_input);
	g_triggered_obj = NULL;
	while(gb_tk_state == TK_STATE_IDLE) {
		g_triggered_obj = ui_process_events(ctx->display);
		if(if_card_state(ctx->cctx) != 0)  {
			gb_tk_state = TK_STATE_CARD_DISCONNECTED;
			break;
		}
#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON
		tk_usb_callback(ctx, gba_apdu_data_field, 0);
#endif
		os_wait(67);			//14~15 fps
		if(g_triggered_obj != NULL) break;
	}
	OS_DEBUG_EXIT();
	return g_triggered_obj;
}

uint16 tk_sim_handler(void * h, sim_query_p query) {
	tk_context_p ctx = (tk_context_p)h;
	uint16 len = 0;
	datetime dtime;
	uint8 buffer[64];
	if(strcmp((const char *)query->cmd, "esid") == 0) {
		len = SHARD_ESID_SIZE;
		len = (query->buf_size < len)?query->buf_size:len;
		memcpy(query->buffer, ctx->esid, len);
	} else if(strcmp((const char *)query->cmd, "now") == 0) {
		if_time_get(&dtime);
		tk_iso8601_encode(&dtime, buffer);
		len = strlen((const char *)buffer);
		len = (query->buf_size < len)?query->buf_size:len;
		memcpy(query->buffer, buffer, len);
	}
	return len;
}

void tk_task_kron() {
	OS_DEBUG_ENTRY(tk_task_kron);
	tk_context_p ctx;
	uint8 atrlen;
	uint8 atr[36];
	ui_object logo;
	ui_object * signal;
	ui_object * obj;
	ui_alert * alert;
	uint32 csr;
	uint16 i;
	uint8 net_result;
	uint8 switcher = 0;
	void * handle;
	ctx = os_get_context();
	//context initialization
	tk_load_configuration(ctx);			//load configuration from non-volatile memory
	//configure timer
	if_timer_init(ctx->display);			//timer init
	if_delay(30);
	//configure PWM interface
	if_pwm_init(ctx->display);
	sim_register_interface((uint8 *)"kron", ctx, tk_sim_handler);		//register SIM interface
	if_delay(200);
	//initialize screen
	if(ctx->interfaces & TK_INTERFACE_LCD) {
		if_gui_init(ctx->display, ctx->config->orientation, ctx->config->brightness);				//display init (must be called after system init)
	}
	//configure touchscreen interface
	if((ctx->interfaces & (TK_INTERFACE_TCC | TK_INTERFACE_LCD)) == (TK_INTERFACE_TCC | TK_INTERFACE_LCD)) {
		((gui_handle_p)ctx->display)->touch_config = (ui_config *)ctx->config;
		if_touch_init(ctx->display);			//touch screen initialization
	}
	if(ctx->interfaces & TK_INTERFACE_AUD) {
		if_audio_init(ctx->audio);
	}
	//render logo
	if(ctx->interfaces & TK_INTERFACE_LCD) {	
		os_wait(1000 / ((gui_handle_p)ctx->display)->fps);
		memset(&logo, 0, sizeof(ui_object));
		((ui_rect *)&logo)->w = 120;
		((ui_rect *)&logo)->h = 62;
		((ui_rect *)&logo)->x = (((gui_handle_p)ctx->display)->width - ((ui_rect *)&logo)->w) / 2;
		((ui_rect *)&logo)->y = (((gui_handle_p)ctx->display)->height - ((ui_rect *)&logo)->h) / 2;
		csr = os_enter_critical();
		if(((gui_handle_p)ctx->display)->orientation & 0x01) {		//vertical orientation
			ui_image_render(ctx->display, (ui_rect *)&logo, (uint8 *)image_png_leaf2_logo, sizeof(image_png_leaf2_logo), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		} else {
			ui_image_render(ctx->display, (ui_rect *)&logo, (uint8 *)image_png_leaf2_logo, sizeof(image_png_leaf2_logo), UI_IMAGE_ALIGN_TOP | UI_IMAGE_ALIGN_LEFT);
		}
		os_exit_critical(csr);
		((gui_handle_p)ctx->display)->present(ctx->display);
	}
	if(ctx->interfaces & (TK_INTERFACE_SDC | TK_INTERFACE_HDC)) {			//support sdcard or usb host
		if_storage_init(ctx->devices);
	}
	//configure network module either GSM/WiFi
	if(ctx->interfaces & TK_INTERFACE_NET) {
		net_result = if_net_init(ctx->netctx);				//net init
		//configure net interface
		ctx->netctx->mac[0] = 0x24;
		ctx->netctx->mac[1] = 0x0A;
		ctx->netctx->mac[2] = ctx->esid[12];
		ctx->netctx->mac[3] = ctx->esid[13];
		ctx->netctx->mac[4] = ctx->esid[14];
		ctx->netctx->mac[5] = ctx->esid[15];
		if_net_init_config(ctx->netctx);		//net init
		if_net_set_exception_callback(ctx->netctx, tk_net_exception_handler);
		if_net_set_prepare_context(ctx->netctx, ctx);
		if_net_set_prepare_callback(ctx->netctx, tk_net_prepare_handler);
		if_net_set_finish_context(ctx->netctx, ctx);
		if_net_set_finish_callback(ctx->netctx, tk_net_finish_handler);
		memcpy(ctx->netctx->staip, ctx->config->net_static_ip, 4);				//load ip address
		memcpy(ctx->netctx->stadns1, ctx->config->net_static_dns_1, 4);		//load dns1 address 
		memcpy(ctx->netctx->stadns2, ctx->config->net_static_dns_2, 4);		//load dns2 address
		memcpy(ctx->netctx->name, ctx->config->net_nodename, SHARD_MAX_NODE_NAME);		//load dns2 address
		//check for invalid name
		if(ctx->netctx->name[0] == 0xFF || 
			strlen((const char *)ctx->netctx->name) == 0 || 
			strlen((const char *)ctx->netctx->name) > 15) {
			//generate new name
			sprintf((char *)ctx->netctx->name, "KRON_%02X%02X%02X%02X", 
				ctx->esid[12],ctx->esid[13], ctx->esid[14],ctx->esid[15]);
		}
	}
	//configure usb interface
	if(ctx->interfaces & TK_INTERFACE_USB) {
		if_usb_init(ctx->uctx, ctx, tk_usb_handler);					//usb init (2016.03.28)
		if_delay(50);
	}
	//configure contact card interface
	if(ctx->interfaces & TK_INTERFACE_ICC) {
		if_card_init(ctx->display, ctx->cctx);	//card init
	} else {
		ctx->cctx = NULL;		//clear card context;
	}
	//configure NFC/RFID interface
	if(ctx->interfaces & TK_INTERFACE_NFC) {
#if (SHARD_NFC_TYPE == 0x522)
		if_rfid_init(ctx->rctx);							//rfid init		(2017.06.17)
#endif
#if (SHARD_NFC_TYPE == 0x532)
		if(if_nfc_init(ctx->rctx) == 0) {
			//init as picc reader
			if_nfc_init_picc(ctx->rctx);
			if(net_result == 0) {
				//network is on, power down NFC, to prevents interference
				if_nfc_sleep(ctx->rctx);
			} else {
				//network not available, enable NFC rf
				//if_nfc_set_rf_field(((tk_context_p)ctx)->rctx, PN532_RF_ENABLE);
				if_nfc_close(((tk_context_p)ctx)->rctx);
			}
		}
#endif
	}
	//configure BLE
	if(ctx->interfaces & TK_INTERFACE_BLE) {
		if(if_ble_init(ctx->bctx, ctx->netctx) == 0) {
			//init success, bluetooth module exist
		}
	}
	//configure GPS
	if(ctx->interfaces & TK_INTERFACE_GPS) {
		if(if_gps_init(ctx->gpsctx, ctx->netctx) == 0) {
			//init success, gps module exist
			if_gps_power_up(ctx->gpsctx);
		}
	}
	//configure UI system
	tk_init_gui(ctx);
	os_create_task(ctx, tk_task_status, "status", 15, 1024);
	os_create_task(ctx, tk_task_render, "render", 32, 16384);
	//os_create_task(ctx, tk_task_worker, "worker", 48, 4096);
	if_delay(200);
	//handle = if_pwm_create(GPIOE, 1, 25, 200);
	//if_pwm_start(handle);
	//first time
	if_display_wake(ctx->display);
	os_wait(200);
	((gui_handle_p)ctx->display)->set_backlight(ctx->display, 1);		//backlight on
	tk_clear_resources(ctx);
	//init netbios for Wi-Fi
	if(ctx->netctx->type == NET_TYPE_WIFI) {
		netbios_init(ctx);
	}
	while(1) {
		g_counter = 0;
			obj = ui_process_events(ctx->display);
			os_wait(67);												//check for user activity
			if(obj == NULL) {
				if(g_counter <= 600) g_counter++;
			} else {
				//an event detected, reset counter
				g_counter = 0;
			}
		
	}
	
	while(1) {
		reset_picc:
		g_counter = 0;
		//picc_connected = 0;
		while(if_card_state(ctx->cctx) != 0)  {
			
			obj = ui_process_events(ctx->display);
			os_wait(67);												//check for user activity
			if(obj == NULL) {
				if(g_counter <= 600) g_counter++;
			} else {
				//an event detected, reset counter
				g_counter = 0;
			}
			//if usb connected, don't sleep
			if(ctx->uctx->instance->dev_state != USBD_STATE_DEFAULT && ctx->uctx->instance->dev_state != USBD_STATE_SUSPENDED) {
				if(g_counter > 600) g_counter = 0;
			}
		}
		//card detected, start disable NFC, and init contact card communication
		if_nfc_sleep(ctx->rctx);
		//re-init card
		if_card_init(ctx->display, ctx->cctx);	//card init
		//card is inserted (released from sleep)
		os_wait(250);		//wait till voltage stabilized
		//if_card_wake(ctx->cctx);
		if((atrlen = if_card_connect(ctx->cctx, SCARD_EXCLUSIVE, atr)) != (uint8)-1) {
			gb_tk_state = TK_STATE_LIST_APP;
			//os_wait(200);
			ctx->uctx->status(ctx->uctx, if_card_state(ctx->cctx));		//set card inserted status
			
			ui_set_toggle_state(ui_get_object_by_name((gui_handle_p)ctx->display, "myCard"), UI_TOGGLE_ON);
			//start autoplay if enabled
			if(ctx->runstate & TK_AUTOPLAY_ENABLED) ctx->runstate |= TK_AUTOPLAY_RUN;
			ctx->offset = SHARD_AUTOREC_START;
			//execute toolkit main loop
			tk_main_loop(ctx);
			ui_set_toggle_state(ui_get_object_by_name((gui_handle_p)ctx->display, "myCard"), UI_TOGGLE_OFF);
			//clear default card manager
			ctx->cmlen = 0;
			memset(ctx->cmaid, 0, TK_MAX_AIDLEN);
			//clear AID buffer
			ctx->aidlen = 0;
			memset(ctx->aidbuf, 0, TK_MAX_AIDLEN);
			//clear card OS configuration
			ctx->cos_owver = 0;
			ctx->cos_totalspace = 0;
			ctx->cos_freespace = 0;
			memset(ctx->cos_config, 0, TK_COS_CONFIG_SIZE);
			ctx->cos_grid = -1;
			ctx->flag &= ~(TK_FLAG_SECURE | TK_FLAG_CARD_DORMANT | TK_FLAG_CARD_SYNCHED);									//clear card flag
			ctx->uctx->status(ctx->uctx, if_card_state(ctx->cctx));		//clear card inserted status
			//clear any resources from previous session
			tk_clear_resources(ctx);
			//clear all screen to last display
			//while(ui_pop_screen((gui_handle_p)ctx->display) != NULL);
			obj = ui_get_screen_by_name(ctx->display, "home") ;
			ui_clear_body(ctx->display, obj);
		} else {
			//invalid card
			alert = ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)"Please re-insert\nthe card", UI_ALERT_INFO, UI_ALERT_ICON_ERROR);
			while(if_card_state(ctx->cctx) == 0) os_wait(40);
			ui_alert_close(ctx->display, alert);
		}
		//clear myCard marker
		obj = ui_get_object_by_name(ctx->display, "myCard") ;
		if(obj != NULL) {
			((ui_icon_tool *)obj)->show_marker = FALSE;
		}
#if (SHARD_NFC_TYPE == 0x522)
		if_rfid_init(ctx->rctx);							//rfid init		(2017.06.17)
#endif
#if (SHARD_NFC_TYPE == 0x532)
		if(if_nfc_init(ctx->rctx) == 0) {
			if_nfc_begin(ctx->rctx);
			os_wait(100);
		}
#endif
		
	}
	OS_DEBUG_EXIT();
}

__ALIGN_BEGIN audio_handle ahandle __attribute__((section(".RW_ERAM2"))) __ALIGN_END;
void tk_main(void)
{
	gui_handle ghandle;
	scard_context scard;
	scard_context * iterator;
	net_context nctx;
	usb_context uctx;
	gps_context gpsctx;
	tk_config conf;
	tk_context tctx;
	owl_context octx;
	bt_context bctx;
#if (SHARD_NFC_TYPE == 0x522)
	rf_context rctx;
#endif
#if (SHARD_NFC_TYPE == 0x532)
	nfc_context rctx;
#endif
	uint8 atr[36];
	uint8 atrlen;
	uint32 wait = 0xFFFFFFFF;
	device_descriptor odev;
	memset(&tctx, 0, sizeof(tk_context));
	memset(&bctx, 0, sizeof(bt_context));
	memset(&nctx, 0, sizeof(net_context));
	memset(&scard, 0, sizeof(scard_context));
#if (SHARD_NFC_TYPE == 0x522)
	memset(&rctx, 0, sizeof(rf_context));
#endif
#if (SHARD_NFC_TYPE == 0x532)
	memset(&rctx, 0, sizeof(nfc_context));
#endif
	memset(&gpsctx, 0, sizeof(gps_context));
	//initialize toolkit context, linking sub contexts to toolkit context
	tctx.flag = 0;			//
	tctx.display = &ghandle;			//display handle
	tctx.audio = &ahandle;			//audio handle
	tctx.cctx = &scard;				//card handle
	tctx.uctx = &uctx;					//usb dev handle
	tctx.netctx = &nctx;				//network handle
	tctx.config = &conf;				//configuration
	tctx.octx = &octx;					//orb-weaver link handle (obsolete)
	tctx.rctx = &rctx;					//nfc handle
	tctx.bctx = &bctx;					//bluetooth handle
	tctx.gpsctx = &gpsctx;				//gps handle
	tctx.app_state = TK_APP_STATE_DORMANT;
	//global system init
	//for(wait;wait!=0;wait--);		//wait first
	if_sdram_init();
	if_flash_init();
#if SHARD_RTOS_ENABLED
	//for(wait=1000000;wait!=0;wait--);		//wait till voltage stabilized
	os_init(&tctx);					
#else
	if_sys_init(NULL);		//system init (must be called first)
	if_delay(280);
#endif
	tctx.devices = dev_create_node(DEV_TYPE_NONE, "/", NULL, NULL, NULL, NULL, NULL, NULL);
	//ui_add_body(&ghandle, ui_qrcode_create(UI_COLOR_BLACK, ((gui_handle_p)&ghandle)->height - 96, "hello world"));
#if SHARD_RTOS_ENABLED
	os_create_task(&tctx, tk_task_kron, "kron", 100, 16384);
	os_start();
	while(1);
#else

	while(1) {
		while(if_card_state(&scard) != 0)  {
			tk_present(&tctx);
		}
		if_delay(100);
		if((atrlen = if_card_connect(&scard, SCARD_EXCLUSIVE, atr)) != (uint8)-1) {
			gb_tk_state = TK_STATE_LIST_APP;
			if_delay(200);
			uctx.status(&uctx, if_card_state(&scard));		//set card inserted status
			
			ui_set_toggle_state(ui_get_object_by_name((gui_handle_p)&ghandle, "myCard"), UI_TOGGLE_ON);
			//start autoplay if enabled
			if(tctx.runstate & TK_AUTOPLAY_ENABLED) tctx.runstate |= TK_AUTOPLAY_RUN;
			tctx.offset = SHARD_AUTOREC_START;
			//execute toolkit main loop
			tk_main_loop(&tctx);
			ui_set_toggle_state(ui_get_object_by_name((gui_handle_p)&ghandle, "myCard"), UI_TOGGLE_OFF);
			//clear default card manager
			tctx.cmlen = 0;
			memset(tctx.cmaid, 0, TK_MAX_AIDLEN);
			//clear AID buffer
			tctx.aidlen = 0;
			memset(tctx.aidbuf, 0, TK_MAX_AIDLEN);
			//clear card OS configuration
			tctx.cos_owver = 0;
			tctx.cos_totalspace = 0;
			tctx.cos_freespace = 0;
			tctx.flag &= ~(TK_FLAG_SECURE | TK_FLAG_CARD_DORMANT | TK_FLAG_CARD_SYNCHED);									//clear card flag
			memset(tctx.cos_config, 0, TK_COS_CONFIG_SIZE);
			tctx.cos_grid = -1;
			uctx.status(&uctx, if_card_state(&scard));				//clear card inserted status
			//clear any resources by previous operation
			tk_clear_resources(ctx);
			//clear all screen to last display
			while(ui_pop_screen((gui_handle_p)&ghandle) != NULL);
		}
	}
#endif
	while(1);
}