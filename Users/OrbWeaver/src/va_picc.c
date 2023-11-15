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

#if SHARD_NFC_TYPE == 0x522
/////////////////////////////////////////PICC APIS////////////////////////////////////////////
static void va_picc_read() {				//param1=context, param2 = block no, param3 =size
	OS_DEBUG_ENTRY(va_picc_read);
	rf_context_p ctx;
	uint16 block = va_o2f(vm_get_argument(1));
	uint16 size = va_o2f(vm_get_argument(2));
	uint16 len;
	uint8 buffer[240];
	uint8 buflen = sizeof(buffer);
	if(((vm_object *)vm_get_argument(0))->len != 0) {
		ctx = ((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->ctx;
		if((ctx->state & (RF_STATE_OPENED | RF_STATE_CONNECTED)) != (RF_STATE_OPENED | RF_STATE_CONNECTED)) goto exit_picc_read;					//check for opened state
		if(if_picc_read(ctx, block, buffer, &buflen) == 0) {
			g_pVaRetval = vm_create_object(buflen, buffer);
		}
	}
	exit_picc_read:
	OS_DEBUG_EXIT();
	return;
}

static void va_picc_write() {				//param1= context, param2 = block no, param3 = payload
	OS_DEBUG_ENTRY(va_picc_write);
	rf_context_p ctx;
	uint16 block = va_o2f(vm_get_argument(1));
	vm_object * payload = vm_get_argument(2);
	if(((vm_object *)vm_get_argument(0))->len == 0) goto exit_picc_write;
	ctx = ((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->ctx;
	if((ctx->state & (RF_STATE_OPENED | RF_STATE_CONNECTED)) != (RF_STATE_OPENED | RF_STATE_CONNECTED)) goto exit_picc_write;					//check for opened state
	if_picc_write(ctx, block, payload->bytes, payload->len);
	exit_picc_write:
	OS_DEBUG_EXIT();
	return;
}

static void va_picc_close() {				//param1 = context
	OS_DEBUG_ENTRY(va_picc_close);
	rf_context_p ctx;
	if(((vm_object *)vm_get_argument(0))->len != 0) {
		ctx = ((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->ctx;
		ctx->state &= ~RF_STATE_OPENED; 
	}
	OS_DEBUG_EXIT();
}

void va_picc_open() {						//param = none
	OS_DEBUG_ENTRY(va_picc_open);
	rf_context_p ctx;
	uint16 ret = -1;
	va_picc_context defctx;
	if(g_pfrmctx == NULL) goto exit_picc_open;
	ctx = g_pfrmctx->rctx;
	if((ctx->state & RF_STATE_CONNECTED) == 0) goto exit_picc_open;		//no rfid currently connected
	//if(ctx->state & RF_STATE_OPENED) return;					//already opened
	ctx->state |= RF_STATE_OPENED;
	((va_default_context *)&defctx)->ctx = ctx;
	((va_default_context *)&defctx)->close = va_picc_close;
	((va_default_context *)&defctx)->read = va_picc_read;
	((va_default_context *)&defctx)->write = va_picc_write;
	((va_default_context *)&defctx)->offset =  0;
	((va_default_context *)&defctx)->seek = NULL;
	defctx.state = 0;
	defctx.keys = NULL;
	vm_set_retval(vm_create_object(sizeof(va_picc_context), &defctx));
	VM_ARG->picc_ctx = vm_get_retval()->bytes;
	exit_picc_open:
	OS_DEBUG_EXIT();
	return;
}

void va_picc_auth() {				//param1 = context, param2= block, param3 = key_id, param4 = key (hexstring)
	OS_DEBUG_ENTRY(va_picc_auth);
	uint8 binkey[64];
	uint8 hexkey[129];
	uint8 len;
	rf_context_p ctx = ((va_default_context *)((vm_object *)vm_get_argument(0))->bytes)->ctx;
	uint16 block = va_o2f(vm_get_argument(1));
	uint16 key_id = va_o2f(vm_get_argument(2));
	vm_object * key = vm_get_argument(3);
	memset(hexkey, 0, sizeof(hexkey));
	len =  (key->len > 128)?128:key->len;
	memcpy(hexkey, key->bytes, len);
	tk_hex2bin(hexkey, binkey);
	g_pVaRetval = vm_load_bool(if_picc_authenticate(ctx, PICC_CMD_MF_AUTH_KEY_A + key_id, block, binkey) == 0);
	OS_DEBUG_EXIT();
	return;
}

void va_picc_transmit() {				//param1 = context, param2= block, param3 = key_id, param4 = key (bytes)
	OS_DEBUG_ENTRY(va_picc_auth);
	uint8 binkey[64];
	uint8 hexkey[129];
	uint8 len;
	OS_DEBUG_EXIT();
	return;
}
#endif


#if SHARD_NFC_TYPE == 0x532
static va_picc_key * va_picc_create_key(void * ctx, uint8 keyid, uint8 block, uint8 * auth) {
	va_picc_key * key = (va_picc_key *)os_alloc(sizeof(va_picc_key));
	if(key == NULL) return NULL;
	key->ctx = ctx;
	key->keyid = keyid;
	key->block = block;
	memcpy(key->key, auth, 6);
	key->next = NULL;
	return key;
}

static void va_picc_add_key(va_picc_context * ctx, va_picc_key * key) {
	va_picc_key * prev_key = NULL;
	va_picc_key * iterator = ctx->keys;
	va_picc_key * temp;
	if(iterator == NULL) ctx->keys = key;
	else {
		while(iterator->next != NULL) {
			iterator= iterator->next;
		}
		iterator->next = key;
	}
}

static void va_picc_mifare_auth(va_picc_context * ctx, uint8 block) {
	uint8 apdu[64];
	uint8 resBuffer[64];
	uint8 resLen;
	nfc_context_p nctx;
	va_picc_key * iterator = ctx->keys;
	while(iterator != NULL) {
		if(iterator->ctx == ctx && iterator->block == block) {
			apdu[0] = iterator->keyid;
			apdu[1] = iterator->block;
			memcpy(apdu + 2, iterator->key, 6);
			nctx = ((nfc_context_p)(((va_default_context *)ctx)->ctx));
			memcpy(apdu + 8, nctx->uid, nctx->uidLen);
			if_nfc_data_exchange(nctx, apdu, nctx->uidLen + 8, resBuffer, &resLen);
		}
		iterator = iterator->next;
	}
}

static void va_picc_release_keys(va_picc_context * ctx) {
	va_picc_key * prev_key = NULL;
	va_picc_key * iterator = ctx->keys;
	va_picc_key * temp;
	while(iterator != NULL) {
		if(iterator->ctx == ctx) {
			if(prev_key == NULL) { 
				ctx->keys = iterator->next;
			} else {
				prev_key->next = iterator->next;
			}
			temp = iterator;
			iterator = iterator->next;
			os_free(temp);
		} else {
			prev_key = iterator;
			iterator = iterator->next;
		}
	}
}

static void va_picc_release_all_keys(va_picc_context * ctx) {
	va_picc_key * prev_key = NULL;
	va_picc_key * iterator = ctx->keys;
	va_picc_key * temp;
	if(ctx == NULL) return;
	iterator = ctx->keys;
	while(iterator != NULL) {
		temp = iterator;
		iterator = iterator->next;
		os_free(temp);
	}
	ctx->keys = NULL;
}

static void va_picc_seek(VM_DEF_ARG) {
	OS_DEBUG_ENTRY(va_picc_seek);			//NFC API
	uint16 block = va_o2f(vm_get_argument(VM_ARG, 1));
	uint16 len;
	if(((vm_object *)vm_get_argument(VM_ARG, 0))->len == 0) goto exit_picc_seek;
	//set sector to seek
	((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->offset = block;
	exit_picc_seek:
	OS_DEBUG_EXIT();
	return;
}

static void va_picc_read(VM_DEF_ARG) {				//param1=context, param2 = block no, param3 =size
	OS_DEBUG_ENTRY(va_picc_read);			//NFC API
	nfc_context_p rctx;
	uint8 apdu[64];
	uint8 apduLen;
	uint8 resbuffer[262];
	uint8 resLen = 255;		//amount of buffer available for response
	uint16 block;
	va_picc_context * pctx;
	uint16 size = va_o2f(vm_get_argument(VM_ARG, 1));
	uint16 len;
	if(((vm_object *)vm_get_argument(VM_ARG, 0))->len == 0) goto exit_picc_read;
	rctx = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
	if((rctx->nfc_state & NFC_STATE_CONNECTED) == 0) goto exit_picc_read;		//no rfid currently connected
	//if((rctx->nfc_state & (NFC_STATE_OPENED | NFC_STATE_CONNECTED)) != (NFC_STATE_OPENED | NFC_STATE_CONNECTED)) goto exit_picc_read;					//check for opened state
	//if(if_picc_read(rctx, block, buffer, &buflen) == 0) {
	block = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->offset;
	pctx = ((va_picc_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes);
	if(if_nfc_open(rctx) != 0) goto exit_picc_read;	
	switch(rctx->type) {
		case NFC_MIFARE:
			if_nfc_list_passive_target(rctx, 2, PN532_MIFARE_ISO14443A, NULL, 0);
			apdu[0] = MIFARE_CMD_READ;
			apdu[1] = block;
			memcpy(apdu + 8, g_pfrmctx->rctx->uid, g_pfrmctx->rctx->uidLen);
			apduLen = 2;
			va_picc_mifare_auth(pctx, block);
			if(if_nfc_data_exchange(g_pfrmctx->rctx, apdu, apduLen, resbuffer, &resLen) == 0) {
				vm_set_retval(vm_create_object(resLen, resbuffer));
			}
			break;
	}
	if_nfc_close(rctx);
	exit_picc_read:
	OS_DEBUG_EXIT();
	return;
}

static void va_picc_write(VM_DEF_ARG) {				//param1= context, param2 = block no, param3 = payload
	OS_DEBUG_ENTRY(va_picc_write);			//NFC API
	nfc_context_p rctx;
	uint8 apdu[64];
	uint8 apduLen;
	uint8 resbuffer[262];
	uint16 block;
	va_picc_context * pctx;
	uint8 resLen = 255;		//amount of buffer available for response
	vm_object * payload = vm_get_argument(VM_ARG, 1);
	if(((vm_object *)vm_get_argument(VM_ARG, 0))->len == 0) goto exit_picc_write;
	rctx = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
	if((rctx->nfc_state & NFC_STATE_CONNECTED) == 0) goto exit_picc_write;		//no rfid currently connected
	//if((ctx->nfc_state & (NFC_STATE_OPENED | NFC_STATE_CONNECTED)) != (NFC_STATE_OPENED | NFC_STATE_CONNECTED)) goto exit_picc_write;					//check for opened state
	//if(if_picc_read(ctx, block, buffer, &buflen) == 0) {
	block = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->offset;
	pctx = ((va_picc_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes);
	if(if_nfc_open(rctx) != 0) goto exit_picc_write;	
	switch(rctx->type) {
		case NFC_MIFARE:
			if_nfc_list_passive_target(rctx, 2, PN532_MIFARE_ISO14443A, NULL, 0);
			apdu[0] = MIFARE_CMD_WRITE;
			apdu[1] = block;
			memcpy(apdu + 2, payload->bytes, payload->len);
			apduLen = 2 + payload->len;
			va_picc_mifare_auth(pctx, block);
			if(if_nfc_data_exchange(g_pfrmctx->rctx, apdu, apduLen, resbuffer, &resLen) == 0) {
				vm_set_retval(vm_create_object(resLen, resbuffer));
			}
			break;
	}
	if_nfc_close(rctx);
	exit_picc_write:
	OS_DEBUG_EXIT();
	return;
}

static void va_picc_close(VM_DEF_ARG) {				//param1 = context
	OS_DEBUG_ENTRY(va_picc_close);		//NFC API
	nfc_context_p rctx;
	uint8 i;
	va_picc_context * pctx;
	if(((vm_object *)vm_get_argument(VM_ARG, 0))->len != 0) {
		rctx = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
		pctx = (va_picc_context *)(((vm_object *)vm_get_argument(VM_ARG, 0))->bytes);
		va_picc_release_keys(pctx);
		os_wait(100);
	}
	OS_DEBUG_EXIT();
}

void va_picc_open(VM_DEF_ARG) {						//param = none
	OS_DEBUG_ENTRY(va_picc_open);			//NFC API
	rf_context_p rctx;
	uint16 ret = -1;
	uint8 i;
	va_net_context defctx;
	if(g_pfrmctx == NULL) goto exit_picc_open;
	//if(if_nfc_list_passive_target(g_pfrmctx->rctx, 2, PN532_MIFARE_ISO14443A, NULL, 0) != 0) goto exit_picc_open;		//no rfid currently connected
	if((g_pfrmctx->rctx->nfc_state & NFC_STATE_CONNECTED) == 0) goto exit_picc_open;		//no rfid currently connected
	//g_pfrmctx->rctx->nfc_state |= NFC_STATE_OPENED;
	//if(if_nfc_open(g_pfrmctx->rctx) != 0) goto exit_picc_open;	
	((va_default_context *)&defctx)->ctx = g_pfrmctx->rctx;
	((va_default_context *)&defctx)->close = va_picc_close;
	((va_default_context *)&defctx)->read = va_picc_read;
	((va_default_context *)&defctx)->write = va_picc_write;
	((va_default_context *)&defctx)->offset =  0;
	((va_default_context *)&defctx)->seek = va_picc_seek;
	//clear keylist and state
	((va_picc_context *)&defctx)->state = 0;
	//((va_picc_context *)&defctx)->keys = NULL;
	vm_set_retval(vm_create_object(sizeof(va_picc_context), &defctx));
	//g_picc_context[i] = (va_picc_context *)g_pVaRetval->bytes;
	exit_picc_open:
	OS_DEBUG_EXIT();
	return;
}

void va_picc_auth(VM_DEF_ARG) {				//param1 = context, param2= block, param3 = key_id, param4 = key (bytes)
	OS_DEBUG_ENTRY(va_picc_auth);			//NFC API
	nfc_context_p rctx;
	uint8 apdu[64];
	uint8 apduLen;
	uint8 resbuffer[262];
	uint8 resLen = 255;
	uint8 len;
	uint8 mfCmd;
	va_picc_context * pctx;
	uint16 block = va_o2f(vm_get_argument(VM_ARG, 1));
	uint16 key_id = va_o2f(vm_get_argument(VM_ARG, 2));
	vm_object * key = vm_get_argument(VM_ARG, 3);
	len =  (key->len > 6)?6:key->len;
	if(((vm_object *)vm_get_argument(VM_ARG, 0))->len == 0) goto exit_picc_auth;
	rctx = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
	//if((ctx->nfc_state & (NFC_STATE_OPENED | NFC_STATE_CONNECTED)) != (NFC_STATE_OPENED | NFC_STATE_CONNECTED)) goto exit_picc_auth;					//check for opened state
	
	if(if_nfc_open(rctx) != 0) goto exit_picc_auth;	
	switch(rctx->type) {
		case NFC_MIFARE:
			mfCmd = apdu[0] = (key_id)?MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
			apdu[1] = block;
			memcpy(apdu + 2, key->bytes, 6);
			memcpy(apdu + 8, g_pfrmctx->rctx->uid, g_pfrmctx->rctx->uidLen);
			apduLen = 8 + g_pfrmctx->rctx->uidLen;
			if(if_nfc_data_exchange(g_pfrmctx->rctx, apdu, apduLen, resbuffer, &resLen) == 0) {
				pctx = (va_picc_context *)(((vm_object *)vm_get_argument(VM_ARG, 0))->bytes);
				va_picc_add_key(pctx, va_picc_create_key(((vm_object *)vm_get_argument(VM_ARG, 0))->bytes, mfCmd, block, key->bytes));
				vm_set_retval(vm_load_bool(TRUE));
			} else {
				vm_set_retval(vm_load_bool(FALSE));
			}
		break;
	}
	if_nfc_close(rctx);
	exit_picc_auth:
	OS_DEBUG_EXIT();
	return;
}

void va_picc_transmit(VM_DEF_ARG) {		//transmit raw apdu, param1 = context, param2 = apdu command bytes, return response bytes
	OS_DEBUG_ENTRY(va_picc_transmit);			//NFC API
	nfc_context_p rctx;
	uint8 resbuffer[262];
	uint8 resLen = 255;
	vm_object * apdu;
	if(((vm_object *)vm_get_argument(VM_ARG, 0))->len == 0) goto exit_nfc_transmit;
	ctx = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
	//if((ctx->nfc_state & (NFC_STATE_OPENED | NFC_STATE_CONNECTED)) != (NFC_STATE_OPENED | NFC_STATE_CONNECTED)) goto exit_nfc_transmit;					//check for opened state
	if(if_nfc_open(rctx) != 0) goto exit_nfc_transmit;	
	apdu = (vm_object *)vm_get_argument(VM_ARG, 1);
	if(apdu->len == 0) goto exit_nfc_transmit;
	if(apdu->len < 2) goto exit_nfc_transmit;	//invalid apdu header
	if_nfc_data_exchange(rctx, apdu->bytes, apdu->len, resbuffer, &resLen);
	if(resLen != 0) vm_set_retval(vm_create_object(resLen, resbuffer));
	if_nfc_close(rctx);
	exit_nfc_transmit:
	OS_DEBUG_EXIT();
}

//called one time every execution

void va_picc_init(VM_DEF_ARG) {
	//nothing, picc_context will only be assigned during picc_open
	VM_ARG->picc_ctx = NULL;
}

void va_picc_release_all(VM_DEF_ARG) {
	//release all keys stored on list (only keys, don't release context)
	if(VM_ARG == NULL) return;
	va_picc_release_all_keys(VM_ARG->picc_ctx);
	VM_ARG->picc_ctx = NULL;
}
#endif
////////////////////////////////////END OF PICC APIS//////////////////////////////////////////

