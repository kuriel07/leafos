#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint8 gb_tk_state;
#define gba_apdu_data_field (gba_apdu_buffer + 5)  
extern uint8 gba_net_buffer[NET_BUFFER_SIZE];
extern uint16 gba_orc_size;
extern uint8 gba_grid[20];

#if SHARD_RTOS_ENABLED
void HAL_Delay(uint32 Delay) {
	os_wait(Delay);
}
#endif

#if SHARD_USB_DRIVER == SHARD_DRIVER_OKEY
uint16 tk_usb_handler(void * ctx, uint8 * c_apdu, uint16 len, uint8 * r_apdu) {
	r_apdu[0] = 0x6D;
	r_apdu[1] = 0x00;
	return 2;
}
#endif

#if SHARD_USB_DRIVER == SHARD_DRIVER_KRON

uint8 g_baProCmdBuf[258];
uint8 g_u8ProCmdLen;
uint16 tk_usb_handler(void * ctx, uint8 * cmdbuf, uint16 clen, uint8 * resbuf) {
	OS_DEBUG_ENTRY(tk_usb_handler);
	uint16 rlen = 0;
	tk_context_p tctx = (tk_context_p)ctx;
	if(clen == 0) goto exit_usb_handler;
	tctx->ucmd = cmdbuf[0];
	tctx->ulen = cmdbuf[1];
	tk_memcpy(tctx->ubuf, cmdbuf + 2, cmdbuf[1]);
	tk_usb_exec(ctx);
	rlen = 0;
	exit_usb_handler:
	OS_DEBUG_EXIT();
	return rlen;
}

//tk_usb_exec should always be called only during tk_present
void tk_usb_exec(tk_context_p ctx) {
	OS_DEBUG_ENTRY(tk_usb_exec);
	uint8 resbuf[258];
	//uint32 oid;
	uint8 i = 0;
	uint8 reslen = 0;
	usb_context_p uctx;
	
	if(ctx == NULL) goto exit_usb_exec;
	uctx = ctx->uctx;
	if(uctx == NULL) goto exit_usb_exec;
	if(ctx->ucmd == TK_UCMD_NONE) goto exit_usb_exec;
	resbuf[0] = ctx->ucmd;
	switch(ctx->ucmd) {
		case TK_UCMD_STATUS:						//terminal status
			resbuf[1] = 4;									//length
			resbuf[2] = if_card_state(ctx->cctx);
			resbuf[3] = ((MAJOR_VERSION << 4) | MINOR_VERSION);
			resbuf[4] = MICRO_VERSION;
			resbuf[5] = SHARD_OWL_SUPPORTED;
			reslen = 6;
		send_result:
			ctx->ucmd = TK_UCMD_NONE;			//handled, no need to execute callback
			uctx->write(uctx, resbuf, reslen);			
			break;
		case TK_UCMD_INFO:							//card info
			if(if_card_state(ctx->cctx) != 0) {			//check for inserted card
				resbuf[1] = 0;
				reslen = 2;
				goto send_result;
			}
			resbuf[2] = 0x80;
			resbuf[3] = 1;
			resbuf[4] = ctx->cos_owver;
			resbuf[5] = 0x87;
			resbuf[6] = 4;
			memcpy(resbuf + 7, ctx->cos_config, 4);
			resbuf[11] = 0xF0;
			resbuf[12] = 4;
			resbuf[13] = (uint8)(ctx->cos_totalspace >> 24);
			resbuf[14] = (uint8)(ctx->cos_totalspace >> 16);
			resbuf[15] = (uint8)(ctx->cos_totalspace >> 8);
			resbuf[16] = ctx->cos_totalspace ;
			resbuf[17] = 0xFE;
			resbuf[18] = 4;
			resbuf[19] = (uint8)(ctx->cos_freespace >> 24);
			resbuf[20] = (uint8)(ctx->cos_freespace >> 16);
			resbuf[21] = (uint8)(ctx->cos_freespace >> 8);
			resbuf[22] = ctx->cos_freespace;
			resbuf[23] = 0x4F;
			resbuf[24] = ctx->cmlen;
			memcpy(resbuf + 25, ctx->cmaid, ctx->cmlen);
			ctx->ucmd = TK_UCMD_NONE;			//handled, no need to execute callback
			resbuf[1] = 23 + ctx->cmlen;
			reslen = 25 + ctx->cmlen;
			goto send_result;
		//card operation command
		case TK_UCMD_LIST_APP:						//handled by reader
			if(if_card_state(ctx->cctx) != 0) {			//check for inserted card
				resbuf[1] = 0;
				reslen = 2;
				goto send_result;
			}
			gb_tk_state = TK_STATE_CARD_DISCONNECTED;		//should list application first
			break;
		//card operation command
		case TK_UCMD_SELECT_APP:
			if(if_card_state(ctx->cctx) != 0) {			//check for inserted card
				resbuf[1] = 0;
				reslen = 2;
				goto send_result;
			}
			gb_tk_state = TK_STATE_USB_SELECT; //back to list app
			break;
		case TK_UCMD_RESULT:
		case TK_UCMD_TEXT:
		case TK_UCMD_ITEM:
			if(if_card_state(ctx->cctx) != 0) {			//check for inserted card
				resbuf[1] = 0;
				reslen = 2;
				goto send_result;
			}
			gb_tk_state = TK_STATE_USB_COMMAND;
			break;
		case TK_UCMD_DOWNLOAD_APP:
			resbuf[1] = 1;	
			resbuf[2] = 0xFF;
			reslen = 3;
			ctx->ucmd = TK_UCMD_NONE;			//handled, no need to execute callback
			if(((gui_handle_p)ctx->display)->status & UI_STATUS_WAIT) goto send_result;			//case display is busy, wait user response
			if(ctx->ulen < 4) goto send_result;
			if(if_card_state(ctx->cctx) == 0) {
				tk_river_usb_download(ctx, ctx->ubuf, ctx->ulen);
				resbuf[2] = 0;
			}
			goto send_result;
		case TK_UCMD_DELETE_APP:
			resbuf[1] = 1;	
			resbuf[2] = 0xFF;
			reslen = 3;
			ctx->ucmd = TK_UCMD_NONE;			//handled, no need to execute callback
			if(((gui_handle_p)ctx->display)->status & UI_STATUS_WAIT) goto send_result;			//case display is busy, wait user response
			if(ctx->ulen < 4) goto send_result;
			if(if_card_state(ctx->cctx) == 0) {
				tk_app_delete(ctx, ctx->ubuf, ctx->ulen);
				resbuf[2] = 0;
				gb_tk_state = TK_STATE_USB_DELETE_APP;
			}
			goto send_result;
		default: break;
	}
	exit_usb_exec:
	OS_DEBUG_EXIT();
	return;
}

//tk usb callback should always be called during tk_list_application, tk_setup_menu
void tk_usb_callback(tk_context_p ctx, uint8 * response, uint8 len) {
	OS_DEBUG_ENTRY(tk_usb_callback);
	static uint8 resbuf[258];
	uint8 status  = 0;
	uint8 rlen;
	uint8 clen = 0;
	usb_context_p uctx;
	uint8 tag;
	uint16 size, wsize;
	uint16 i,j;
	uint8 proCmd;
	static uint8 action_counter = 0;
	if(ctx == NULL) goto exit_usb_callback;
	uctx = ctx->uctx;
	if(uctx == NULL) goto exit_usb_callback;
	if(ctx->ucmd == TK_UCMD_NONE) goto exit_usb_callback;
	resbuf[0] = ctx->ucmd;
	switch(ctx->ucmd) {
		case TK_UCMD_LIST_APP:			//(list application) tk_list_application
			if(gb_tk_state != TK_STATE_LIST_APP) goto exit_usb_callback;
			ctx->ucmd = TK_UCMD_NONE;			//handled, no need to execute callback
			tk_memcpy(g_baProCmdBuf, response, len);					//copy to proactive command buffer for next tk_usb_exec
			g_u8ProCmdLen = len;
			//tk_memcpy(resbuf + 2, response, len);
			action_counter = 0;
			//check for matched autoplay AID
			for(i=0,j=0; i<g_u8ProCmdLen;) {
				i += tk_pop(g_baProCmdBuf + i, &tag, &size, resbuf + 4 + j);
				if(tag != 0x61) continue;
				tk_pop(resbuf + 4 + j, &tag, &wsize, resbuf + 4 + j);
				if(tag != 0x4F) continue;
				resbuf[2 + j++] = tag;
				resbuf[2 + j++] = wsize;
				j += wsize;
			}
			resbuf[1] = j;
			uctx->write(uctx, resbuf, j  + 2);		
			goto exit_usb_callback;
		case TK_UCMD_SELECT_APP:		//(setup menu) tk_setup_menu
			if(gb_tk_state != TK_STATE_LIST_MENU) goto exit_usb_callback;
			ctx->ucmd = TK_UCMD_NONE;			//handled, no need to execute callback
			tk_memcpy(g_baProCmdBuf, response, len);					//copy to proactive command buffer for next tk_usb_exec
			tk_pop(g_baProCmdBuf, &tag, &size, g_baProCmdBuf);
			g_u8ProCmdLen = size;
			goto dispatch_host;
		//ucmd communication tag (handled by card)
		case TK_UCMD_RESULT:
			//wait for user input
			status = ctx->ubuf[0];
			//construct terminal response
			clen = tk_response(status, clen, gba_apdu_data_field);
			break;
		case TK_UCMD_TEXT:
			//wait for user input
			status = 0;
			rlen = ctx->ulen;
			tk_memcpy(resbuf + 1, ctx->ubuf, rlen);
			rlen += 1;
			resbuf[0] = 0x04;
			//construct terminal response
			clen = tk_response(status, clen, gba_apdu_data_field);
			//if(status != STK_RES_TERMINATED) { 
			clen += tk_push(gba_apdu_data_field + clen, STK_TAG_TEXT_STRING, rlen, resbuf);
			break;
		case TK_UCMD_ITEM:
			for(i =0; i<g_u8ProCmdLen; ) {
				i += tk_pop(g_baProCmdBuf + i, &tag, &size, resbuf);
				if((tag & 0x7F) == STK_TAG_ITEM) {
						resbuf[size] = 0;
						if(memcmp(resbuf + 1, ctx->ubuf, ctx->ulen) == 0) {
							clen = tk_response(0x00, clen, gba_apdu_data_field);
							clen = tk_response(status, clen, gba_apdu_data_field);
							clen += tk_push(gba_apdu_data_field + clen, STK_TAG_ITEM_ID, 1, resbuf);
							if(action_counter == 0) {
								//use envelope menu
								clen = tk_push(gba_apdu_data_field, ENV_TAG_MENU, clen, gba_apdu_data_field);
							}
							goto dispatch_proactive_command;
						}
				}
			}	
			goto exit_handler;						//item not found
		default:
		exit_handler:									//either an error or transaction terminated/aborted
			resbuf[0] = TK_UCMD_NONE;
			resbuf[1] = 0;
			uctx->write(uctx, resbuf, 2);
			goto exit_usb_callback;
	}
	dispatch_proactive_command:
	//everything should be handled by now
	ctx->ucmd = TK_UCMD_NONE;		//handled
	action_counter++;
	//dispatch pre-constructed command response
	if((rlen = tk_dispatch(ctx, clen, gba_apdu_data_field)) == 0) {
		//action_counter = 0;
		gb_tk_state = TK_STATE_IDLE;
		ctx->runstate &= TK_AUTOPLAY_ENABLED;												//disable autorun/autorec
		ctx->offset = SHARD_AUTOREC_START;
		goto exit_handler;
	} else {
		tk_memcpy(g_baProCmdBuf, response, rlen);					//copy to proactive command buffer for next tk_usb_exec
		//g_u8ProCmdLen = rlen;
		tk_pop(g_baProCmdBuf, &tag, &size, g_baProCmdBuf);
		g_u8ProCmdLen = size;
		gb_tk_state = TK_STATE_EXECUTE;
		//stir up tags
		//tk_memcpy(resbuf + 2, g_baProCmdBuf, size);
		dispatch_host:
		for(i =0,j=0; i<g_u8ProCmdLen; ) {
			i += tk_pop(g_baProCmdBuf + i, &tag, &size, resbuf + 3 + j);
			if((tag & 0x7F) == STK_TAG_CMD_DETAIL) {
				proCmd = resbuf[4 + j];
			}
			if((tag & 0x7F) == STK_TAG_ITEM) {
				//tk_memcpy(resbuf + 4 + j, resbuf + 3 + j, size - 1);
				resbuf[2 + j++] = (tag & 0x7F);
				resbuf[2 + j++] = (size - 1);
				j += (size - 1);
			}
			if((tag & 0x7F) == STK_TAG_TEXT_STRING) {
				//remove dcs
				resbuf[2 + j++] = (tag & 0x7F);
				resbuf[2 + j++] = (size - 1);
				j += (size - 1);
			}
			if((tag & 0x7F) == STK_TAG_ALPHA) {
				tk_memcpy(resbuf + 4 + j, resbuf + 3 + j, size);
				resbuf[2 + j++] = (tag & 0x7F);
				resbuf[2 + j++] = size;
				j += size;
			}
		}
		resbuf[0] = proCmd;				//proactive command tag
		resbuf[1] = j;						//total length
		uctx->write(uctx, resbuf, j + 2);
	}
	exit_usb_callback:
	OS_DEBUG_EXIT();
	return;
}
#endif