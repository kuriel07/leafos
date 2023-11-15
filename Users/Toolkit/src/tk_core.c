#ifdef WIN32
//#include "stdafx.h"
#endif
#ifndef CR_APIS__H
#include "..\..\crypto\inc\cr_apis.h"
#endif
#ifndef IF_APIS__H
#include "..\..\interfaces\inc\if_apis.h"
#endif
#ifndef TK_APIS__H
#include "..\..\toolkit\inc\tk_apis.h"
#endif
#include "..\..\defs.h"
#include "..\..\config.h"
#include <stdio.h>
#include <string.h>
#include "wolfssl\wolfcrypt\sha256.h"

BYTE gba_apdu_buffer[TK_BUFFER_SIZE];
#define gba_apdu_data_field (gba_apdu_buffer + 5) 

void tk_memcpy(BYTE * dst, BYTE * src, WORD size) _REENTRANT_ {
	WORD i;
	if(dst == src) return;
	if(size == 0) return;
  	if(dst > src) {
		for(i=size;i>0;) {
			i--;
		  	dst[i] = src[i];
		}
	} else {
		for(i=0;i<size;i++) {
			dst[i] = src[i];
		}
	}
}

BYTE tk_push(BYTE * buffer, BYTE tag, BYTE length, BYTE * value) _REENTRANT_ {
	BYTE i;//, j;
	BYTE hlen = 2;
	if(length > 0x7F) {
		hlen = 3;
	}
	if((buffer + hlen) > value) {
		//i = length + hlen;
		tk_memcpy(buffer + hlen, value, length);	
		i = 0;	 
		buffer[i++] = tag;
		if(length > 0x7F) {
	 		buffer[i++] = 0x81;
			buffer[i++] = length;
		} else {
			buffer[i++] = length;
		}
		i += length;
	} else { 
		i = 0;	  
		buffer[i++] = tag;
		if(length > 0x7F) {
	 		buffer[i++] = 0x81;
			buffer[i++] = length;
		} else {
			buffer[i++] = length;
		}
		
		tk_memcpy(buffer + i, value, length);
		i += length;
	}
	return i;	
}

//return size of header (TAG+LENGTH)
uint8 tk_pop(BYTE * buffer, BYTE * tag, WORD * size, BYTE * value) _REENTRANT_ {
	uint16 i = 0; 
	uint16 j;
	uint16 lsize = 0;
	tag[0] = (BYTE)buffer[i++];
	if(buffer[i] == 0x81) {
		i++;
	} else if(buffer[i] == 0x82) {
		i++;
		lsize = (WORD)buffer[i++];
		lsize <<= 8;
	} 
	lsize += (WORD)buffer[i++];
	for(j=0;j<lsize;j++) {
		value[j] = buffer[i++];
	}
	size[0] = lsize;
	return i;							//return size of preceding header (TAG+LENGTH)
}

WORD tk_pop_by_tag(BYTE * buffer_in, BYTE size, BYTE tag, BYTE * buffer_out) _REENTRANT_ {
   	WORD i = 0, j;
	BYTE ttag;
	WORD tsize;
	BYTE cntr = 0;
	uint8 temp_buffer[256];
	if(size == (BYTE)-1) return -1;
	//use tag number
	if((tag & 0xF0) == 0xF0) {
	 	cntr = (tag & 0x0F);  
		for(j = 0; j < cntr; j++) i += tk_pop(buffer_in + i, &ttag, &tsize, temp_buffer);
		memcpy(buffer_out, temp_buffer, tsize);
		return tsize;
	}
	//use tag id
	while(i < size) {
		i += tk_pop(buffer_in + i, &ttag, &tsize, temp_buffer);
		ttag &= 0x7F;
		if(ttag == (tag & 0x7F)) {
			memcpy(buffer_out, temp_buffer, tsize);
			return tsize;
		}
	}
	//tag not found
	return -1;
} 

WORD tk_build_apdu(BYTE * buffer, BYTE cla, BYTE ins, BYTE p1, BYTE p2, BYTE le) {
	buffer[0] = cla;
	buffer[1] = ins;
	buffer[2] = p1;
	buffer[3] = p2;
	buffer[4] = le;
	return 5;
}

WORD tk_build_apdu_data(BYTE * buffer, BYTE cla, BYTE ins, BYTE p1, BYTE p2, BYTE lc, BYTE * dfield) {
	tk_build_apdu(buffer, cla, ins, p1, p2, lc);
	memcpy(buffer + 5, dfield, lc);
	return 5 + lc;
}
	

WORD tk_get_status(BYTE * data, WORD len) {
	if(len < 2 && len < 0) return -1;		//invalid length
	return (((WORD)data[len-2]<<8) | data[len-1]);
}

uint16 tk_card_send(tk_context_p ctx, uint8 * c_apdu, uint16 clen, uint8 * r_apdu, uint16 *rlen) {
	uint8 cpdu[6];
	uint16 tlen;
	uint16 status;
	uint16 len;
	uint8 ins;
	ins = c_apdu[1];
	memcpy(cpdu, c_apdu, 5);
	tlen = if_card_send(ctx->cctx, c_apdu, clen, r_apdu);
	if(SCARD_ERROR(tlen)) return -1;
	status = tk_get_status(r_apdu, tlen);
	switch((ctx->cctx->protocol & 0x0F)) {
		case ICC_PROTOCOL_T0:
			if(tlen > 2) break;
			if((status & 0xFF00) == 0x9100) {				//toolkit proactive command fetch
				len = (status & 0xFF);
				//fetch proactive command
				tlen = if_card_send(ctx->cctx, cpdu, tk_build_apdu(cpdu, 0x00, 0x12, 0x00, 0x00, len), r_apdu);
				if(SCARD_ERROR(tlen)) return (uint16)-1;
				goto preprocess_fetched_data;
			} else if((status & 0xFF00) == 0x6100 || status == 0x62F1) {			//data chaining/response available
				len = (status & 0xFF);
				if(status == 0x62F1) len = 0x100;
				tlen = if_card_send(ctx->cctx, cpdu, tk_build_apdu(cpdu, 0x00, 0xC0, 0x00, 0x00, len), r_apdu);
				if(SCARD_ERROR(tlen)) return (uint16)-1;
				if(status != 0x62F1) status = tk_get_status(r_apdu, tlen);		//next data chain exist (don;t return current status)
			} else if((status & 0xFF00) == 0x6C00 && status > 0x6C00) {		//invalid length, fetch the right length
				cpdu[4] = status & 0xFF;
				tlen = if_card_send(ctx->cctx, cpdu, 5, r_apdu);
				if(SCARD_ERROR(tlen)) return (uint16)-1;
				status = tk_get_status(r_apdu, tlen);
			}
			break;
		case ICC_PROTOCOL_T1: 
			if((status & 0xFF00) == 0x9000) {				//toolkit proactive command fetch
				//len = (status & 0xFF);
				//fetch proactive command
				//tlen = if_card_send(ctx->cctx, cpdu, tk_build_apdu(cpdu, 0x00, 0x12, 0x00, 0x00, len), r_apdu);
				//if(SCARD_ERROR(tlen)) return -1;
			preprocess_fetched_data:
				status = tk_get_status(r_apdu, tlen);
				if(ins != 0x14 && ins != 0xC2 && ins != 0x10) break;
				if(tlen <= 2) break;
				if(ctx->flag & TK_FLAG_SECURE) {
					tlen = tk_unwrap_response_buffer(ctx, r_apdu, tlen - 2);
					rlen[0] = tlen;
					return status;
				}
			} 
			break;
	}
	rlen[0] = tlen - 2;
	return status;
}


uint16 tk_unwrap_response_buffer(tk_context_p ctx, uint8 * buffer, uint16 length) _REENTRANT_ {
	//BYTE len = length;				//Lc
	//BYTE paddedLen;
	cr_context cryptoCtx;
	uint8 ccrc[4], hcrc[4];
	//use cipher operation only, no integrity checking
	if(ctx->flag & TK_FLAG_SECURE) {			//pandora key
		//decrypt padded data first	(not including first five bytes, CLA+INS+P1+P2+Lc)
		cr_init_context(&cryptoCtx, buffer);
		cryptoCtx.mode = CR_MODE_DES2 | CR_MODE_CBC | CR_MODE_ENCRYPT;		//use padding 0 (actual equal to padding 1), algo 3
		cryptoCtx.key = ctx->pkey;					//use authenticated pkey (pandora)
		cr_do_crypt(&cryptoCtx, 0, length);		
		//calculate actual Lc 
		//len = length;
		while(buffer[--length] != 0x80 && length > 0); 		//detect padding method 2, fixed when cmac containing 80h (2016.05.05)
		if(length == 0) return -1;
		//backup response crc
		length = length - 4;
		memcpy(ccrc, buffer + length, 4);
		//calculate crc result of deciphered message
		cr_init_crc(&cryptoCtx, buffer);
		cr_calc_crc(&cryptoCtx, 0, length, hcrc);
		if(memcmp(ccrc, hcrc, 4) != 0) return -1;
	}				  
	return length;	
}

uint16 tk_wrap_data_buffer(tk_context_p ctx, uint8 * buffer, uint16 length) _REENTRANT_ {
	uint16 len = length;				//Lc
	//BYTE paddedLen;
	uint16 lc;
	cr_context cryptoCtx;
	uint8 crc[4];
	//use cipher operation only, no integrity checking
	if(ctx->flag & TK_FLAG_SECURE) {			//pandora key
		cr_init_crc(&cryptoCtx, buffer);
		cr_calc_crc(&cryptoCtx, 0, length, crc);
		//add crc result
		memcpy(buffer + length, crc, 4);		
		length += 4;
		
		//add padding data with padding method 2, if data % 8 == 0 then add 8 bytes padding
		lc = ((length + 0x08) & 0xF8);
		memset(buffer + length, 0, 8);
		buffer[length] = 0x80;

		cr_init_context(&cryptoCtx, buffer);
		cryptoCtx.mode = CR_MODE_DES2 | CR_MODE_CBC | CR_MODE_ENCRYPT;		//use padding 0 (actual equal to padding 1), algo 3
		cryptoCtx.key = ctx->pkey;					//use authenticated pkey (pandora)
		cr_do_crypt(&cryptoCtx, 0, lc);	
		//lc += 4;
		//calculate actual Lc 
		len = lc;
		//while(buffer[--len] != 0x80); 		//detect padding method 2, fixed when cmac containing 80h (2016.05.05)
	}				  
	return len;	
}

void tk_text2ip(uint8 * text, uint8 ip[4]) {
	
}

//configuration interface
void tk_get_config(tk_config * config) {
	if_flash_data_read(NULL, 0, (uint8 *)config, sizeof(tk_config));
}

void tk_set_config(tk_config * config) {
	if_flash_data_write(NULL, 0, (uint8 *)config, sizeof(tk_config));
}


uint8 tk_gen_hash(uint8 * buffer, uint16 len, uint8 * signature) {
	Sha256 sha_ctx;
	uint8 b_len;
	uint8 sha_chksum[SHARD_HASH_SIZE];
	memset(signature, 0, SHARD_HASH_SIZE);		//zero memory
	memset(buffer + len, 0, SHARD_HASH_SIZE);
	//orclen += sgnlen;
	//orclen &= 0xFFE0;
	wc_InitSha256(&sha_ctx);
	wc_Sha256Update(&sha_ctx, (const uint8 *)buffer, len);
	wc_Sha256Final(&sha_ctx, sha_chksum);
	memcpy(signature, sha_chksum, SHARD_HASH_SIZE);
	return SHARD_HASH_SIZE;
}






