#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\config.h"
#include "wolfssl\wolfcrypt\sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint8 gb_tk_state;
#define gba_apdu_data_field (gba_apdu_buffer + 5)  
extern uint8 gba_net_buffer[NET_BUFFER_SIZE];
extern uint16 gba_orc_size;

static void tk_orbriver_category_select(ui_object * cbox, void * params) {
	gb_tk_state = TK_STATE_RIVER_CAT_CHANGED;
}

static void tk_orbriver_exit_click(ui_object * item, void * params) {
	//river exit button click
	gb_tk_state = TK_STATE_LIST_APP;		//back to list application
}

static void tk_orbriver_search_click(ui_object * item, void * params) {
	//river search button click
	gb_tk_state = TK_STATE_RIVER_SEARCH;
}

static void tk_orbriver_search_query_click(ui_object * item, void * params) {
	gb_tk_state = TK_STATE_RIVER_SEARCH_QUERY;
}

static void tk_orbriver_item_click(ui_object * item, void * params) {
	//when river item clicked (start download immdtly)
	ui_orbriver_item * witem = (ui_orbriver_item *)item;
	switch(witem->event) {
		case UI_ITEM_EVENT_CLICK:
			gb_tk_state = TK_STATE_RIVER_DOWNLOAD;		//download current river item
			break;
		case UI_ITEM_EVENT_DETAIL:
			ui_item_show_detail(params, (ui_orbriver_item *)item);
			break;
	}
}


static uint8 tk_orc_check_hash(uint8 * orcfile, uint16 orclen, uint8 * signature, uint8 sgnlen) {
	Sha256 sha_ctx;
	uint8 b_len;
	uint8 sha_chksum[SHARD_HASH_SIZE];
	memset(orcfile + orclen, 0, sgnlen);		//zero memory
	//orclen += sgnlen;
	//orclen &= 0xFFE0;
	wc_InitSha256(&sha_ctx);
	wc_Sha256Update(&sha_ctx, (const uint8 *)orcfile, orclen);
	wc_Sha256Final(&sha_ctx, sha_chksum);
	return memcmp(sha_chksum, signature, sgnlen);
}

uint8_c * g_strErrorMsg[] = {
	(uint8_c *)"Install Success",
	(uint8_c *)"Network failed",
	(uint8_c *)"Authentication failed",
	(uint8_c *)"Install [for load] failed",
	(uint8_c *)"Install [for install] failed",
	(uint8_c *)"Load failed",
	(uint8_c *)"ORC verification failed",
	(uint8_c *)"ORC exceeded",
	(uint8_c *)"Hash invalid",
};

//static uint8 g_baDownloadAid[33] = { 0, 0 };

void tk_river_usb_download(tk_context_p ctx, uint8 * aid, uint8 len) {
	//sprintf((char *)g_baDownloadOid, "%ld", oid);
	//tk_bin2hex(aid, len, g_baDownloadAid);
	if(len > 0x10) return;
	memcpy(ctx->aidbuf, aid, len);
	ctx->aidlen = len;
	gb_tk_state = TK_STATE_RIVER_USB_DOWNLOAD;
}

void tk_river_loop(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_river_loop);
	uint8 url_buffer[256];
	uint8 search_buffer[40] = { 0, 0 };
	uint8 app_name[TK_MAX_AIDLEN];
	uint16 j;
	uint16 size, isize;
	uint8 tag;
	uint8 oid[16];
	uint32 nod;
	uint16 bnum;
	uint16 osize;		//orblet size
	uint8 name[16];
	uint8 desc[SHARD_MAX_DESC + 1];
	uint8 elfBuffer[17];
	uint8 appBuffer[17];
	uint8 sdBuffer[17];
	uint8 esid_string[33];
	uint8 hash_buffer[SHARD_HASH_SIZE];
	uint8 grid[16];
	uint16 i, sz, fsz;
	uint8 prefSecLevel;
	uint8 * extHdr;
	uint8 extHdrLen;
	uint8 category = 0;		//history
	ui_object * obj;
	ui_object * btn;
	ui_alert * alert;
	ui_textbox * textbox;
	ui_list_scroll * listbox;
	ssl_cert_p cert = NULL;
	net_request req;
	gui_handle_p display;
	ssl_handle_p ssdl;
	ui_object * cbbox;
	ui_object * scbox;
	uint16 hh;
	uint8 error_code = 0;
	uint8 fromUsb = FALSE;
	//get esid_string from current context
	tk_bin2hex(ctx->esid, SHARD_ESID_SIZE, esid_string);
	sprintf((char *)grid, "%d", ctx->cos_grid);
	while(1) {
		restart_display:
		ui_clear_body(ctx->display, NULL);
		switch(gb_tk_state) {
			case TK_STATE_RIVER_USB_DOWNLOAD:
				fromUsb = TRUE;
				if(ctx->aidlen == 0) goto exit_river;							//no AID on buffer
				tk_bin2hex(ctx->aidbuf, ctx->aidlen, search_buffer);
				sprintf((char *)url_buffer, "http://orbleaf.com/apis/river/dl.php?aid=%s&esid=%s", search_buffer, esid_string);
				ctx->aidlen = 0;
				memset(ctx->aidbuf, 0, TK_MAX_AIDLEN);
				gb_tk_state = TK_STATE_RIVER_DOWNLOAD;
				goto start_download;
				break;
			case TK_STATE_RIVER:			//list application
				ui_add_body(ctx->display, (cbbox = ui_combo_create(category, 3, (uchar *)"", tk_orbriver_category_select, "History", "Latest", "Most Downloads")));
				ui_set_object_name((ui_object *)cbbox, "cbRiverTab");
				i = 0;
				display = (gui_handle_p)ctx->display;
				hh = ((ui_rect *)display->body)->h - ((ui_rect *)cbbox)->h;
				listbox = (ui_list_scroll *)ui_list_create(0, ((ui_rect *)cbbox)->h, ((ui_rect *)display->body)->w, hh);
				ui_add_body(ctx->display, listbox);
				ui_list_add((ui_object *)listbox, obj = (ui_object *)ui_item_create((uchar *)"Search", NULL, 0, 0, tk_orbriver_search_click));
				ui_set_object_name((ui_object *)obj, "txRiverSrch");
				if(tk_history_count(NULL) == 0) category = 1;		//set to latest
				//list top most orblet
				if(category > 0) {
					sprintf((char *)url_buffer, "http://orbleaf.com/apis/river/ls.php?cat=%d&grid=%s&esid=%s&q=%s", (category - 1), grid, esid_string, search_buffer);
					alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Connecting...", UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
					//wake wifi
					if_net_wake(ctx->netctx);
					//sending reqeuest to river service (list application)
#if 0	//SHARD_SSL_SUPPORT
					//gba_orc_size = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, 0, gba_net_buffer);
					gba_orc_size = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, NULL, 0, gba_net_buffer);
#else			
					net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert);
					gba_orc_size = http_send(ctx->netctx, &req, NULL, NULL, 0, gba_net_buffer);
#endif
					//network sleep
					if_net_sleep(ctx->netctx);
					//decode base64 charset
					gba_orc_size = tk_base64_decode(gba_net_buffer, gba_orc_size);
					ui_alert_close(ctx->display, alert);
				
					while(i < gba_orc_size) {
						i+= tk_pop(gba_net_buffer + i, &tag, &size, gba_net_buffer);
						if(tag == 0x30 && size != 0) {		//valid sequence tag
							for(j=0;j<size;) {
								j += tk_pop(gba_net_buffer + j, &tag, &isize, gba_net_buffer);
								gba_net_buffer[isize] = 0;
								switch(tag) {
									case 0x02: 		//OID
										strncpy((char *)oid, (const char *)gba_net_buffer, sizeof(oid));
										break;
									case 0x08:		//orblet size
										sz = atoi((const char *)gba_net_buffer);
										break;
									case 0x0A:		//NOD
										nod = atoi((const char *)gba_net_buffer);
										break;
									case 0x05: 		//Name
										strncpy((char *)name, (const char *)gba_net_buffer, sizeof(name));
										break;
									case 0x0B:		//build number
										bnum = atoi((const char *)gba_net_buffer);
										break;
									case 0x0C:		//hash
										memcpy(hash_buffer, gba_net_buffer, isize);
										break;
									case 0x0D:		//description
										strncpy((char *)desc, (const char *)gba_net_buffer, sizeof(desc));
										break;
								}
							}
							ui_list_add((ui_object *)listbox, (obj = (ui_object *)ui_orbriver_item_create(name, oid, sz, nod, bnum, hash_buffer, desc, tk_orbriver_item_click)));
						}
					}
				} else {
					//recent history
					for(i=0;i<4;i++) {
						if(tk_history_read(NULL, i, (tk_app_record *)gba_net_buffer) == 0) {
							ui_list_add((ui_object *)listbox, (obj = (ui_object *)ui_orbriver_item_create(
								((tk_app_record *)gba_net_buffer)->app_name, 
								((tk_app_record *)gba_net_buffer)->oid, 
								((tk_app_record *)gba_net_buffer)->size, 
								((tk_app_record *)gba_net_buffer)->counter, 
								((tk_app_record *)gba_net_buffer)->bnum, 
								((tk_app_record *)gba_net_buffer)->hash, 
								((tk_app_record *)gba_net_buffer)->desc, 
								tk_orbriver_item_click)));
						}
					}
				}
				//refresh list view
				ui_reinitialize_object((ui_object *)listbox);
				break;
			case TK_STATE_RIVER_ERROR:
				if(fromUsb == TRUE) {
					ctx->uctx->status(ctx->uctx, 0x40 | error_code);		//send error code
					gb_tk_state = TK_STATE_CARD_DISCONNECTED;
					goto exit_river;
				}
				alert = ui_alert_show(ctx->display, (uint8 *)"Error", (uint8 *)g_strErrorMsg[error_code], UI_ALERT_BUTTON_OK | UI_ALERT_BUTTON_CANCEL, UI_ALERT_ICON_ERROR);
				gb_tk_state = TK_STATE_RIVER;
				break;
			case TK_STATE_RIVER_SEARCH:
				ui_add_body(ctx->display, (ui_object *)(textbox = (ui_textbox *)ui_textbox_create(UI_TEXTBOX_ALPHANUM, (uint8 *)"", 63, 1)));
				ui_add_body(ctx->display, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"OK", STK_RES_SUCCESS, tk_orbriver_search_query_click)));
				obj->target = textbox;
				ui_add_body(ctx->display, (obj = (ui_object *)ui_button_create(UI_COLOR_WHITE, (uchar *)"Cancel", STK_RES_TERMINATED, tk_orbriver_search_query_click)));
				obj->target = textbox;
				break;
			case TK_STATE_RIVER_DOWNLOAD:
				sprintf((char *)url_buffer, "http://orbleaf.com/apis/river/dl.php?oid=%s&esid=%s", search_buffer, esid_string);
				start_download:
				//sending download request
				alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Authenticating...", UI_ALERT_INFO, UI_ALERT_ICON_SECURE);
				if(gp_authenticate(ctx, prefSecLevel, search_buffer, hash_buffer, &fsz) != 0) {
					ui_alert_close(ctx->display, alert);
					//cannot authenticate card
					search_buffer[0] = 0;
					error_code = TK_ERR_AUTH;
					gb_tk_state = TK_STATE_RIVER_ERROR;
					goto restart_display;
					//break;
				}
				ui_alert_close(ctx->display, alert);
				//push history
				tk_history_push(NULL, search_buffer, app_name, desc, bnum, osize, hash_buffer);
				alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Downloading...", UI_ALERT_INFO, UI_ALERT_ICON_NETWORK);
				//device wake
				if_net_wake(ctx->netctx);
#if 0	//SHARD_SSL_SUPPORT
				gba_orc_size = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, NULL, 0, gba_net_buffer);
#else			
				net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert);
				gba_orc_size = http_send(ctx->netctx, &req, NULL, NULL, 0, gba_net_buffer);
#endif
				//device sleep
				if_net_sleep(ctx->netctx);
				ui_alert_close(ctx->display, alert);
				if(gba_orc_size == 0) {
					error_code = TK_ERR_NETWORK;
					gb_tk_state = TK_STATE_RIVER_ERROR;
					goto restart_display;
				}
				//decode base64 string
				gba_orc_size = tk_base64_decode(gba_net_buffer, gba_orc_size);
				gba_orc_size = fsz;																					//set to actual length receive during gp_authenticate
				if(tk_orc_check_hash(gba_net_buffer, gba_orc_size, hash_buffer, SHARD_HASH_SIZE) != 0) {			//use SHA256
					//invalid hash
					search_buffer[0] = 0;
					error_code = TK_ERR_ORC_DATA;
					gb_tk_state = TK_STATE_RIVER_ERROR;
					goto restart_display;	
				}
				//decapsulate from TLV header
				tk_pop(gba_net_buffer, &tag, &size, gba_apdu_data_field);
				if(tag != 0x6F) {
					//invalid header
					search_buffer[0] = 0;
					error_code = TK_ERR_ORC_DATA;
					gb_tk_state = TK_STATE_RIVER_ERROR;
					goto restart_display;
				}
				if(gba_apdu_data_field[0] > 0x01) {
					//invalid version
					search_buffer[0] = 0;
					error_code = TK_ERR_ORC_DATA;
					gb_tk_state = TK_STATE_RIVER_ERROR;
					goto restart_display;
				}
				//ORC security level = gba_apdu_data_field[4]
				prefSecLevel = gba_apdu_data_field[2];
				memcpy(elfBuffer, gba_apdu_data_field + 6, 17);
				memcpy(appBuffer, gba_apdu_data_field + 7 + elfBuffer[0], 17);
				memcpy(sdBuffer, gba_apdu_data_field + 8 + elfBuffer[0] + appBuffer[0], 17);
				//extended header
				extHdr = gba_apdu_data_field + 9 + elfBuffer[0] + appBuffer[0] + sdBuffer[0];
				extHdrLen = size - (9 + elfBuffer[0] + appBuffer[0] + sdBuffer[0]);
				//encapsulate ORC file
				gba_orc_size = gp_push(gba_net_buffer, 0xE1, gba_orc_size, gba_net_buffer);
				//print_hex(gba_net_buffer, gba_orc_size);
				
				//install [for load]
				if(gp_install(ctx, 0x02, elfBuffer + 1, elfBuffer[0]) != 0) {
					ui_alert_close(ctx->display, alert);
					error_code = TK_ERR_INSTALL_ORC;
					gb_tk_state = TK_STATE_RIVER_ERROR;
					goto restart_display;
					//break;
				}
				i = 0;
				//load data
				ui_alert_close(ctx->display, alert);
				alert = ui_alert_show(ctx->display, (uint8 *)"Information", (uint8 *)"Loading...\nDo not remove card", UI_ALERT_PROGRESS_BAR, UI_ALERT_ICON_INFO);
				//should add some delay in case smart card didn't ready (2016.04.05)
				//if_delay(200);
				while(i < gba_orc_size) {
					sz = ((i + 0x80) > gba_orc_size)? (gba_orc_size - i): 0x80;
					if(gp_load(ctx, i / 0x80, gba_net_buffer + i, sz) != 0) {
						ui_alert_close(ctx->display, alert);
						error_code = TK_ERR_LOAD_ORC;
						gb_tk_state = TK_STATE_RIVER_ERROR;
						goto restart_display;
						//break;
					}
					ui_alert_set_progress_bar(ctx->display, alert, (uint32)((uint32)i * 100) / gba_orc_size);
					i += 0x80;
					//if_delay(200);
				}
				//prepare install [for install]
				i = 0;
				tk_memcpy(gba_apdu_data_field, elfBuffer, elfBuffer[0] + 1);		//copy ELF AID
				i += (elfBuffer[0] + 1);
				gba_apdu_data_field[i++] = 0;		//length of module AID
				tk_memcpy(gba_apdu_data_field + i, appBuffer, appBuffer[0] + 1);	//copy APP AID
				i += (appBuffer[0] + 1);
				gba_apdu_data_field[i++] = 0;		//privilleged
				gba_apdu_data_field[i++] = 0;		//install parameters
				gba_apdu_data_field[i++] = 0;		//install token
				//install [for install]
				//printf("installing orblet\n");
				if(gp_install(ctx, 0x04, gba_apdu_data_field, i) != 0) {
					ui_alert_close(ctx->display, alert);
					error_code = TK_ERR_INSTALL_APP;
					gb_tk_state = TK_STATE_RIVER_ERROR;
					goto restart_display;
					//break;
				}
				
				delete_elf_file:			//automatically deleted by card
				ui_alert_close(ctx->display, alert);
				search_buffer[0] = 0;
				if(fromUsb == TRUE) {
					ctx->uctx->status(ctx->uctx, 0x40 | error_code);		//send success result
					gb_tk_state = TK_STATE_CARD_DISCONNECTED;
					goto exit_river;
				} else {
					gb_tk_state = TK_STATE_RIVER;
				}
				goto restart_display;
				break;
			default: break;
		}
		gb_tk_state = TK_STATE_IDLE;
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
		if(gb_tk_state == TK_STATE_CARD_DISCONNECTED) goto exit_river;
		switch(gb_tk_state) {
			case TK_STATE_RIVER: break;
			case TK_STATE_RIVER_SEARCH: 
				ui_push_screen(ctx->display, NULL);
				break;
			case TK_STATE_RIVER_CAT_CHANGED: 
				process_category:
				cbbox = (ui_object *)ui_get_object_by_name(ctx->display, "cbRiverTab");
				category = ((ui_combo *)cbbox)->index;
				gb_tk_state = TK_STATE_RIVER;
				break;
			case TK_STATE_RIVER_ERROR:
				ui_alert_close(ctx->display, alert);
				gb_tk_state = TK_STATE_RIVER;
				break;
			case TK_STATE_RIVER_SEARCH_QUERY:
				//start search query
				textbox = (ui_textbox *)obj->target;
				search_buffer[0] = 0;		//null terminated string
				strcpy((char *)search_buffer, (const char *)textbox->content);
				ui_pop_screen(ctx->display);
				//gb_tk_state = TK_STATE_RIVER;
				goto process_category;
			case TK_STATE_RIVER_DOWNLOAD: 
				//clear keystate
				((ui_object *)obj)->state &= ~UI_STATE_KEYDOWN;
				sprintf((char *)url_buffer, "install %s?", ((ui_text *)obj)->text);
				//show confirmation dialog
				alert = ui_alert_show(ctx->display, (uint8 *)"Confirmation", (uint8 *)url_buffer, UI_ALERT_BUTTON_OK | UI_ALERT_BUTTON_CANCEL, UI_ALERT_ICON_INFO);
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
				ui_alert_close(ctx->display, alert);
				if(gb_tk_state == TK_STATE_CARD_DISCONNECTED) break;
				if(ui_alert_result(ctx->display) == UI_ALERT_RESULT_OK) {
					//copy OID of selected item
					strcpy((char *)search_buffer, (const char *)((ui_orbriver_item *)obj)->oid);
					strcpy((char *)app_name, ((ui_text *)obj)->text);
					osize = ((ui_orbriver_item *)obj)->size;
					bnum = ((ui_orbriver_item *)obj)->bnum;
					strncpy((char *)desc, (const char *)((ui_orbriver_item *)obj)->desc, SHARD_MAX_DESC);
					
					gb_tk_state = TK_STATE_RIVER_DOWNLOAD;
				} else {
					//operation cancelled
					gb_tk_state = TK_STATE_RIVER;
				}
				break;
			case TK_STATE_CARD_DISCONNECTED: goto exit_river;
			default: 
				goto exit_river;
		}
	}
	exit_river:
#if 0	//SHARD_SSL_SUPPORT
	//if(ssdl != NULL) if_ssl_close(ssdl);
	if_ssl_release(ctx->netctx);
#endif
	OS_DEBUG_EXIT();
	return;
}

uint8 tk_history_count(void * handle) {
	OS_DEBUG_ENTRY(tk_history_count);
	uint8 i = 0;
	tk_app_record temp;
	for(i=0;i<4;i++) {
		if(tk_history_read(NULL, i, &temp) != 0) break;
	}
	OS_DEBUG_EXIT();
	return i;
}

uint8 tk_history_read(void * handle, uint8 index, tk_app_record * record) {
	OS_DEBUG_ENTRY(tk_history_read);
	uint8 stat = 0;
	index = index % 4;
	if_flash_data_read(NULL, SHARD_HISTORY_START + (index * sizeof(tk_app_record)), (uint8 *)record, sizeof(tk_app_record));
	if(record->tag != TK_APPREC_TAG) stat = -1;
	OS_DEBUG_EXIT();
	return stat;
}

void tk_history_push(void * handle, uint8 * oid, uint8 * app_name, uint8 * desc, uint16 bnum, uint16 size, uint8 * hash) {
	OS_DEBUG_ENTRY(tk_history_push);
	tk_app_record * rec = (tk_app_record *)(gba_net_buffer + 16384);
	tk_app_record temp;
	uint8 i,j;
	uint16 lowest_rate = 0xFFFF;
	uint8 cur_idx = 0;
	
	if_flash_data_read(NULL, SHARD_HISTORY_START, (uint8 *)rec, (sizeof(tk_app_record) * 4));
	//increment counter in-case oid already existed
	for(i=0;i<4;i++) {
		if(strncmp((const char *)rec[i].oid, (const char *)oid, 8) == 0) {
			rec[i].counter++;
			break;
		}
	}
	if(i == 4) i = 3;		//put on last record
	cur_idx = i;
	//create new record
	rec[cur_idx].tag = TK_APPREC_TAG;
	memcpy(rec[cur_idx].oid, oid, 8);
	memcpy(rec[cur_idx].app_name, app_name, TK_MAX_AIDLEN);
	memcpy(rec[cur_idx].hash, hash, SHARD_HASH_SIZE);
	strncpy((char *)rec[cur_idx].desc, (const char *)desc, SHARD_MAX_DESC);
	rec[cur_idx].bnum = bnum;
	rec[cur_idx].size = size;
	rec[cur_idx].counter = 1;
	//sort record by counter
	for(j=0;j<3;j++) {
		if(rec[j].counter < rec[j+1].counter) {
			memcpy(&temp, &rec[j+1], sizeof(tk_app_record));
			memcpy(&rec[j+1], &rec[j], sizeof(tk_app_record));
			memcpy(&rec[j], &temp, sizeof(tk_app_record));
		}			
	}
	if_flash_data_write(NULL, SHARD_HISTORY_START, (uint8 *)rec, (sizeof(tk_app_record) * 4));
	OS_DEBUG_EXIT();
}
