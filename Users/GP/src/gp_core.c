#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\defs.h"
#include <stdio.h>
#include <string.h>

#define gba_apdu_data_field (gba_apdu_buffer + 5)  

uint8 gba_host_challange[16];
uint8 gb_keyset = 0;
uint8 gb_scp = 0;
uint8 gb_seclevel = 0;
uint8 gba_senc_key[16];
uint8 gba_cmac_key[16];
uint8 gba_rmac_key[16];
uint8 gba_dek_key[16];
uint8 gba_rmac[8];
uint8 gba_cmac[8];

WORD gp_push(BYTE * buffer, BYTE tag, WORD length, BYTE * value) {
	//TO DO
	WORD i = 0, j;
	BYTE hlen = 2;
	if(length > 0x100) {
		hlen = 4;
		//i = length + hlen;
		tk_memcpy(buffer + hlen, value, length);	
		//i = 0;	 
		buffer[i++] = tag;
	 	buffer[i++] = 0x82;
		buffer[i++] = (length >> 8);
		buffer[i++] = (length & 0xFF);
		i += length;
	} else if(length > 0x7F) { 
		hlen = 3; 
		tk_memcpy(buffer + hlen, value, length);
		buffer[i++] = tag;
	 	buffer[i++] = 0x81;
		buffer[i++] = length;
		
		i += length;
	} else {
		hlen = 2;  
		tk_memcpy(buffer + hlen, value, length);
		buffer[i++] = tag;
	 	buffer[i++] = length;
		
		i += length;
	}
	return i;
}

WORD gp_build_apdu(BYTE * buffer, BYTE cla, BYTE ins, BYTE p1, BYTE p2, BYTE lc) {
	if(gb_seclevel > 0) cla |= 0x04;

	return 5 + 8;
}

WORD gp_build_apdu_data(BYTE * buffer, BYTE cla, BYTE ins, BYTE p1, BYTE p2, BYTE lc, BYTE * dfield) {
	BYTE len;				//Lc
	BYTE paddedLen;
	cr_context crypto;
	BYTE cardCmac[8];
	BYTE hostCmac[8];  
	tk_build_apdu(buffer, cla, ins, p1, p2, lc);
	memcpy(buffer + 5, dfield, lc);
	memset(buffer + 5 + lc, 0, 8 - ((5 + lc) % 8));		//add padding for MAC
	len = buffer[4];
	if(gb_seclevel != 0) {
		buffer[0] |= 0x04;				//add secure channel bit flag
	}
	//print_hex(dfield, 0x10);
	//calculate cmac
	//len += 5;							//include command header
	if(gb_seclevel & 0x01) {			//C-MAC security
		buffer[5 + lc] = 0x80;
	 	//while(buffer[--len] != 0x80);		//calculate actual data length without padding
		//buffer[4] = len + (8 + 1); 			//len = offset to became actual length need to be added by 1, modify command header Lc
		paddedLen = ((lc + 5 + 0x07) & 0xF8);	//calculate total padded length
		memset(buffer + lc + 5, 0, paddedLen - (len + 5));		//set zero memory	
		buffer[5 + lc] = 0x80;								//use padding 2
		//re-calculate icv
		cr_xor(gba_cmac_key, gba_cmac, 8);				//use XOR operation
		buffer[4] = lc + 8;						//padded length w/o command header
		cr_init_mac(&crypto, CR_MAC_ALGO3 | CR_MAC_PAD0 | CR_MAC_CUSTOM_IV, buffer);
		crypto.key = gba_cmac_key;					//use cmac key
		cr_set_iv(&crypto, gba_cmac);			//set custom icv
		cr_calc_mac(&crypto, 0, paddedLen, gba_cmac);
		//if(mmMemCmp(hostCmac, cardCmac, 8) != 0) return -1;		//invalid authentication
		//mmMemCpy(ctx->cMac, cardCmac, 8);				//set custom icv
	}
	//lc = buffer[4];
	//encrypt data
	if(gb_seclevel & 0x02) {			//S-ENC
		//decrypt padded data first	(not including first five bytes, CLA+INS+P1+P2+Lc)
		//memset(buffer + 5 + lc, 0, ((lc + 0x07) & 0xF8) - lc);
		//buffer[5 + lc] = 0x80;
		//if(gb_seclevel & 0x01 && (buffer[4] % 8) != 0) len = buffer[4] - 8;		//actual data length
		lc = ((len + 0x08) & 0xF8);
		memset(buffer + 5 + len, 0, 8);
		buffer[5 + len] = 0x80;
		cr_init_crypt(&crypto, CR_MODE_DES2 | CR_MODE_CBC | CR_MODE_ENCRYPT, buffer + 5); 		//use padding 0 (actual equal to padding 1), algo 3
		crypto.key = gba_senc_key;					//use cmac key	
		cr_do_crypt(&crypto, 0, lc);
		if((len & 0x07) == 0) buffer[4] += 8;		//push padding onto 
		//lc = (lc + 0x07) & 0xF8;
		buffer[4] = lc;
	}
	if(gb_seclevel & 0x01) {			//cmac exist and Lc > 8
		buffer[4] = (lc + 8);					//padded length + mac length
		memcpy(buffer + 5 + lc, gba_cmac, 8);
	}
	lc = buffer[4];
	//printf("Constructed APDU : "); print_hex(buffer, lc + 5);
	return lc + 5;
}

uint8 gp_init_auth(tk_context_p ctx, uint8 seclevel) {
	
}

//generate TEA session key between terminal-server
uint8 gp_generate_tskey(tk_context_p ctx, uint8 * esid) {
	cr_context crypto;
	tk_memcpy(ctx->eak, esid, SHARD_ESID_SIZE);
	memset(ctx->eak, 0, SHARD_ESID_SIZE);
	ctx->eak[0] = 0xEA;
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_CBC | CR_MODE_ENCRYPT, ctx->eak); 
	crypto.key = esid;
	cr_do_crypt(&crypto, 0, 16);
	memset(ctx->ebk, 0, SHARD_ESID_SIZE);
	ctx->ebk[0] = 0xEB;
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_CBC | CR_MODE_ENCRYPT, ctx->ebk); 
	crypto.key = esid;
	cr_do_crypt(&crypto, 0, 16);
	memset(ctx->eck, 0, SHARD_ESID_SIZE);
	ctx->eck[0] = 0xEC;
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_CBC | CR_MODE_ENCRYPT, ctx->eck); 
	crypto.key = esid;
	cr_do_crypt(&crypto, 0, 16);
}

uint8 gp_authenticate(tk_context_p ctx, uint8 seclevel, uint8 * oid, uint8 * hash, uint16 * filesize) {
	uint16 status;
	//BYTE challange[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	uint16 len;
	uint16 rlen;
	uint8 url_buffer[256];
	uint8 div_buffer[20];
	uint8 derivationData[16];
	uint8 esid_buffer[33];
	uint8 tc_buffer[32];
	uint16 i;
	uint8 alen, hash_len;
	ssl_cert_p cert = NULL;
	net_request req;
	cr_context crypto;
	tk_config t_config;
	uint8 cryptogram[32];
	uint8 netbuf[0x400];
	uint8 mac_res[8];
#if SHARD_SSL_SUPPORT
	ssl_handle_p ssdl;
#endif
	memset(derivationData, 0, 16);
	memset(crypto.icv, 0, sizeof(crypto.icv));
	cr_randomize(gba_host_challange, sizeof(gba_host_challange));
	//memset(gba_host_challange, 0, sizeof(gba_host_challange));
	status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu_data(gba_apdu_buffer, 0x80, 0x50, 0x00, 0x00, 0x10, gba_host_challange), gba_apdu_data_field, &rlen);
	//status = tk_get_status(gba_apdu_data_field, len);
	//if((status & 0xFF00) != 0x6100 || status == 0x6100) return -1;
	if(status != 0x9000) return -1;
	//valid global platform card
	//status = tk_card_send(ctx, gba_apdu_buffer, tk_build_apdu(gba_apdu_buffer, 0x80, 0xC0, 0x00, 0x00, status & 0xFF), gba_apdu_data_field, &rlen);
	gb_keyset = gba_apdu_data_field[10];
	gb_scp =  gba_apdu_data_field[11];
	if(gb_scp != 0x06) return -1;			//only support SCP06
	//calculate derivation data
	derivationData[2] = gba_apdu_data_field[12];		//sequence number
	derivationData[3] = gba_apdu_data_field[13];
	
	
	///////////////////////////////       TRIPLE ENTITY AUTHENTICATION        ///////////////////////////////
	//should request key from server here (TEA, Triple Entity Authentication)
	tk_get_config(&t_config);
	tk_base64_encode(ctx->esid, SHARD_ESID_SIZE, esid_buffer);
	gp_generate_tskey(ctx, ctx->esid);
	tk_base64_encode(gba_apdu_data_field, 10, div_buffer);
	tk_memcpy(netbuf + 6, gba_apdu_data_field, 10);
	memset(netbuf, 0, 6);
	netbuf[0] = gb_scp;			//[SCP:1][0:5][DIV:10]   (x)  [EAK:16]   => CRYPTOGRAM
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_ECB | CR_MODE_ENCRYPT, netbuf); 
	crypto.key = ctx->eak;
	cr_do_crypt(&crypto, 0, 16);
	tk_base64_encode(netbuf, 16, cryptogram);
	//device wake
	if_net_wake(ctx->netctx);
#if 0// SHARD_SSL_SUPPORT
	sprintf((char *)url_buffer, (const char *)"https://orbleaf.com/apis/river/xc.php?scp=%d&div=%s&esid=%s&tc=%s&oid=%s", gb_scp, div_buffer, esid_buffer, cryptogram, oid);

	//if(if_ssl_init(ctx->netctx, (uint8 *)orbleaf_cert_der_1024, sizeof(orbleaf_cert_der_1024)) != 0) return -1;
	if((cert = if_ssl_create_cert((uint8 *)orbleaf_cert_der_1024, sizeof(orbleaf_cert_der_1024)) == NULL) return -1;
	//len = https_send(ctx->netctx, IF_HTTP_TYPE_GET, url_buffer, 443, NULL, NULL, 0, netbuf);
	len = https_send(ctx->netctx, net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert), NULL, NULL, 0, netbuf);
	//if_ssl_release(ctx->netctx);
#else
	sprintf((char *)url_buffer, (const char *)"http://orbleaf.com/apis/river/xc.php?scp=%d&div=%s&esid=%s&tc=%s&oid=%s", gb_scp, div_buffer, esid_buffer, cryptogram, oid);
	
	len = http_send(ctx->netctx, net_request_struct(&req, IF_HTTP_TYPE_GET, url_buffer, 80, cert), NULL, NULL, 0, netbuf);
#endif	
	//device sleep (power saving mode)
	if_net_sleep(ctx->netctx);
	if(len == 0) return -1;					//no response from server
	//decode base64 string
	netbuf[len] = 0;
	len = tk_base64_decode(netbuf, len);
	alen = netbuf[0];
	tk_memcpy(netbuf, netbuf+1, len);
	len -= 1;
	//decrypt response from server
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_CBC | CR_MODE_DECRYPT, netbuf); 
	crypto.key = ctx->ebk;
	cr_do_crypt(&crypto, 0, alen);		//decrypt not including mac
	
	//try authenticate message
	tk_memcpy(url_buffer, netbuf + ((alen + 16) & 0xF0), 8);
	//memset(netbuf + alen, 0x00, (len-8) - alen);
	cr_init_crypt(&crypto, CR_MAC_PAD2 | CR_MAC_ALGO3, netbuf);
	crypto.key = ctx->eck;
	cr_calc_mac(&crypto, 0, alen, mac_res);
	if(memcmp(url_buffer, mac_res, 8) != 0) return -1;
	//set length to actual length
	len = alen;
	//encrypt with pkey (from pandora)
	for(i=0;i<len;i+=0x12) {
		cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_ECB | CR_MODE_ENCRYPT, netbuf + i + 2); 
		crypto.key = ctx->pkey;
		cr_do_crypt(&crypto, 0, 16);
	}
	///////////////////////////////   END OF TRIPLE ENTITY AUTHENTICATION    ///////////////////////////////
	
	//for cmac
	derivationData[0] = 0x01;
	derivationData[1] = 0x01;
	memcpy(gba_cmac_key, derivationData, 16);
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_ECB | CR_MODE_ENCRYPT, gba_cmac_key); 
	//default key
	crypto.key = netbuf + 0x14;		//derivated mac key
	cr_do_crypt(&crypto, 0, 16);

	//for rmac
	derivationData[0] = 0x01;
	derivationData[1] = 0x02;
	memcpy(gba_rmac_key, derivationData, 16);
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_ECB | CR_MODE_ENCRYPT, gba_rmac_key); 
	//default key
	crypto.key = netbuf + 0x14;		//derivated mac key
	cr_do_crypt(&crypto, 0, 16);
	//printf("Derivation data : "); print_hex(derivationData, 16);
	//printf("RMAC key : "); print_hex(gba_rmac_key, 16);

	//for senc
	derivationData[0] = 0x01;
	derivationData[1] = 0x82;
	memcpy(gba_senc_key, derivationData, 16);
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_ECB | CR_MODE_ENCRYPT, gba_senc_key); 
	//default key
	crypto.key = netbuf + 0x02;		//derivated enc key
	cr_do_crypt(&crypto, 0, 16);
	//printf("Derivation data : "); print_hex(derivationData, 16);
	//printf("SENC key : "); print_hex(gba_senc_key, 16);

	//for dek
	derivationData[0] = 0x01;
	derivationData[1] = 0x81;
	memcpy(gba_dek_key, derivationData, 16);
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_ECB | CR_MODE_ENCRYPT, gba_dek_key); 
	//default key
	crypto.key = netbuf + 0x26;		//derivated dek key
	cr_do_crypt(&crypto, 0, 16);
	//printf("Derivation data : "); print_hex(derivationData, 16);
	//printf("DEK key : "); print_hex(gba_dek_key, 16);
	hash_len = netbuf[((alen + 16) & 0xF0) + 8]; 
	memcpy(hash, (netbuf + ((alen + 16) & 0xF0) + 8) + 1, 32);
	filesize[0] = (netbuf[((alen + 16) & 0xF0) + 41] * 256) + netbuf[((alen + 16) & 0xF0) + 42];		//actual filesize (big endian)

	//calculate card cryptogram
	memcpy(cryptogram + 16, gba_apdu_data_field + 12, 16);
	memcpy(cryptogram, gba_host_challange, 16);	
	//memset(cryptogram + 16, 0, 8); cryptogram[16] = 0x80;		//padding data
	//calculate card cryptogram
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_CBC | CR_MODE_ENCRYPT, cryptogram); 
	crypto.key = gba_senc_key;
	cr_do_crypt(&crypto, 0, 32);
	//printf("Card Cryptogram : "); print_hex(gba_apdu_data_field + 20, 8);
	//printf("Calculated Card Cryptogram : "); print_hex(cryptogram + 16, 8);

	if(memcmp((const void *)(gba_apdu_data_field + 28), (const void *)(cryptogram + 16), 16) != 0) return -1;	//invalid card cryptogram (cannot authenticate)

	//calculate host cryptogram
	memcpy(cryptogram, gba_apdu_data_field + 12, 16);  
	memcpy(cryptogram + 16, gba_host_challange, 16);	 
	//memset(cryptogram + 16, 0, 8); cryptogram[16] = 0x80;  		//padding data
	//calculate host cryptogram
	cr_init_crypt(&crypto, CR_MODE_AES | CR_MODE_CBC | CR_MODE_ENCRYPT, cryptogram); 
	crypto.key = gba_senc_key;
	cr_do_crypt(&crypto, 0, 32);
	
	//printf("Host Cryptogram : "); print_hex(gba_apdu_data_field + 20, 8);
	//printf("Calculated Host Cryptogram : "); print_hex(cryptogram + 16, 8);

	//memcpy(cryptogram, cryptogram + 16, 8);  			//copy host cryptogram for external authenticate
	//set current MAC for C_MAC and R_MAC
	memset(gba_rmac, 0, 8);
	memset(gba_cmac, 0, 8);

	//external authenticate (should start using secure channel)
	gb_seclevel = seclevel;
	len = gp_build_apdu_data(gba_apdu_buffer, 0x84, 0x82, gb_seclevel, 0x00, 0x10, cryptogram + 16);
	status = tk_card_send(ctx, gba_apdu_buffer, len, gba_apdu_data_field, &rlen);
	//status = tk_get_status(gba_apdu_data_field, len);
	if(status != 0x9000) {
		gb_seclevel = 0;	//reset security level
		return -1;			//card failed to authenticate
	}
	return 0;
}

BYTE gp_install(tk_context_p ctx, BYTE mode, BYTE * aid, BYTE len) {
	WORD status;
	uint16 rlen;
	switch(mode) {
		case 0x02:
			gba_apdu_data_field[0] = len;
			memcpy(gba_apdu_data_field + 1, aid, len);
			len += 1;
			gba_apdu_data_field[len++] = 0;		//sd length
			gba_apdu_data_field[len++] = 0;		//load hash length
			gba_apdu_data_field[len++] = 0;		//load params length
			gba_apdu_data_field[len++] = 0;		//token length
			break;
		case 0x04:
			
			break;
	}
	len = gp_build_apdu_data(gba_apdu_buffer, 0x80, 0xE6, mode, 0x00, len, gba_apdu_data_field);
	status = tk_card_send(ctx, gba_apdu_buffer, len, gba_apdu_data_field, &rlen);
	//status = tk_get_status(gba_apdu_data_field, len);
	//if(status == 0x6581)		//application already installed
	if(status != 0x9000) return -1;
	return 0;
}

BYTE gp_delete(tk_context_p ctx, BYTE * aid, BYTE len) {
	uint16 status;
	uint16 rlen;
	len = tk_push(gba_apdu_data_field, 0x4F, len, aid);
	len = gp_build_apdu_data(gba_apdu_buffer, 0x80, 0xE4, 0x00, 0x00, len, gba_apdu_data_field);
	status = tk_card_send(ctx, gba_apdu_buffer, len, gba_apdu_data_field, &rlen);
	//status = tk_get_status(gba_apdu_data_field, len);
	//if((status & 0xFF00) != 0x6100) return -1;			//card failed to authenticate
	if(status != 0x9000) return -1;
	return 0;

}

BYTE gp_load(tk_context_p ctx, BYTE blocknum, BYTE * buffer, BYTE len) {
	WORD status;
	uint16 rlen;
	len = gp_push(gba_apdu_data_field, 0xC4, len, buffer);		//data block 
	//gp load command
	len = gp_build_apdu_data(gba_apdu_buffer, 0x80, 0xE8, 0x00, blocknum, len, gba_apdu_data_field);
	len = if_card_send(ctx->cctx, gba_apdu_buffer, len, gba_apdu_data_field);
	//status = tk_card_send(ctx->cctx, gba_apdu_buffer, len, gba_apdu_data_field, &rlen);
	status = tk_get_status(gba_apdu_data_field, len);
	if(status != 0x6101) return -1;			//card failed to authenticate
	return 0;
}