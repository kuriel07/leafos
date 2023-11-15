#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\defs.h"
#include "..\..\interfaces\inc\if_apis.h"

static uint8 http_hex2byte(uint8 hexchar) {
	if(hexchar >= 'a' && hexchar <= 'f') return (hexchar - 'a') + 10;
	if(hexchar >= 'A' && hexchar <= 'F') return (hexchar - 'A') + 10;
	if(hexchar >= '0' && hexchar <= '9') return hexchar - '0';
	return 0;
}

static uint16 http_get_chunk_length(uint8 * buffer, uint16 * length) {
	uint16 i =0 ;
	uint16 w, ret = 0;
	while(buffer[i] != '\r') {
		if(buffer[i] == 0) return i;
		w = http_hex2byte(buffer[i++]);
		ret <<= 4;
		ret |= w;
	}
	length[0] = ret;
	return (i+2);
}

uint16 http_parse_response(uint8 * response, uint16 len, uint8 * outbuf) {
	uint16 i = 0;
	uint8 * endptr;
	uint16 dlen = 0;
	uint16 chunklen;
	uint8 clen[10];
	uint16 plen;
	uint8 * ptr;
	uint8 * nextptr;
	endptr = (response + len);		//calculate end of response
	response[len] = 0;				//null string (end of response)
	//compare http header
	if(memcmp(response, "HTTP/1.1 200 OK", 15) != 0) return 0;
	if((ptr = (uint8 *)strstr((char *)response, "Content-Length: ")) != NULL) {
		ptr +=16;
		i = 0;
		while(ptr < (response + len)) {
			clen[i++] = *ptr;
			if(*ptr++ == '\r') break;
		}
		clen[i] = 0;
		dlen = atoi((const char *)clen);
		if(dlen < len) {
			memcpy(outbuf, (response + len) - dlen, dlen);
			len = dlen;
		} else {
			len = 0;		//invalid html
		}
	} else if((ptr = (uint8 *)strstr((char *)response, "Transfer-Encoding: ")) != NULL) {
		ptr +=19;
		if(memcmp(ptr, "chunk", 5) == 0) {
			nextptr = ptr;
			//find end of header
			//while((nextptr = (uint8 *)strstr((char *)ptr, "\r\n")) != ptr+2) ptr = (nextptr + 2);
			if((ptr = (uint8 *)strstr((char *)ptr, "\r\n\r\n")) == NULL) {
				len = 0;
				goto exit_http_parse;
			}
			ptr += 4;
			while(ptr < endptr) {
				ptr += http_get_chunk_length(ptr, &chunklen);
				if(chunklen == 0) break;
				memcpy(outbuf + dlen, ptr, chunklen);
				dlen += chunklen;
				ptr += chunklen;
				ptr += 2;		//skip \r\n
			}
			len = dlen;
		} else {		
			//unsupported transfer encoding
			len = 0;
		}
	} else {
		if((ptr = (uint8 *)strstr((char *)ptr, "\r\n\r\n")) == NULL) {
			len = 0;
			goto exit_http_parse;
		}
		ptr += 4;
		len = len - (ptr - response);
		memcpy(outbuf, ptr, len);
	}
	exit_http_parse:
	return len;
}

//send http request and wait for response
uint16 http_send(net_context_p ctx, net_request_p req, uint8 * headers, uint8 * payload, uint16 length, uint8 * response) {
	OS_DEBUG_ENTRY(http_send);
	//send http request
	uint8 host[210];
	uint8 lbuf[40];
	//contain all the concatenated http-request
	uint8 * cbuf = (gba_net_buffer +0x1000);
	uint16 i = 0;
	uint8 clen[10];
	uint16 plen;
	uint8 * ptr;
	uint8 * nextptr;
	uint8 * endptr;
	uint16 dlen = 0;
	uint16 hdrlen;
	uint16 chunklen;
	uint16 len = 0;
	//check for HTTP prefix
	if(strlen((const char *)req->uri) > sizeof(host)) goto exit_http_send;		//exception, invalid host address
	ptr = net_decode_url(req->uri, &req->port, host);
	switch(req->type & 0x07) {						
		case IF_HTTP_TYPE_HEADER: cbuf[0] = 0; break;
		case IF_HTTP_TYPE_GET: 
			if(length != 0) sprintf((char *)cbuf, "GET %s?%s HTTP/1.1\r\nHost: %s\r\n", ptr, payload, host); 
			else sprintf((char *)cbuf, "GET %s HTTP/1.1\r\nHost: %s\r\n", ptr, host); 
			if(headers != NULL) strcat((char *)cbuf, (const char *)headers);		//additional headers
			strcat((char *)cbuf, "\r\n");		//end header
			break;
		case IF_HTTP_TYPE_DELETE:
			sprintf((char *)cbuf, "DELETE %s HTTP/1.1\r\nHost: %s\r\n", ptr, host); 
			if(headers != NULL) strcat((char *)cbuf, (const char *)headers);		//additional headers
			strcat((char *)cbuf, "\r\n");		//end header
			break;
		case IF_HTTP_TYPE_PUT:
			nextptr = (uint8 *)strstr((char *)ptr, (const char *)"?");
			if(nextptr != NULL) nextptr[0] = 0;		//delete host argument (GET??)
			//check for any additional headers
			if(headers == NULL)
				sprintf((char *)cbuf, "PUT %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\n", ptr, host, length); 
			else 
				sprintf((char *)cbuf, "PUT %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\n%s", ptr, host, length, headers); 
			
			strcat((char *)cbuf, "\r\n");		//end header	
			//calculate header length
			hdrlen = strlen((const char *)cbuf);
			//copy payload
			memcpy(cbuf + hdrlen, payload, length);
			strcat((char *)cbuf, "\r\n");
			break;
		case IF_HTTP_TYPE_POST: 
			nextptr = (uint8 *)strstr((char *)ptr, (const char *)"?");
			if(nextptr != NULL) nextptr[0] = 0;		//delete host argument (GET??)
			//check for any additional headers
			if(headers == NULL)
				sprintf((char *)cbuf, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\n", ptr, host, length); 
			else 
				sprintf((char *)cbuf, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\n%s", ptr, host, length, headers); 
			
			strcat((char *)cbuf, "\r\n");		//end header	
			//calculate header length
			hdrlen = strlen((const char *)cbuf);
			//copy payload
			memcpy(cbuf + hdrlen, payload, length);
			strcat((char *)cbuf, "\r\n");
			break;
	}
	//calculate total length of request packet (2016.06.27)
	length = strlen((const char *)cbuf);
	if(req->type & IF_TRANSPORT_UDP) {					//use preconfigured transport level
		len = if_net_udp_transmit(ctx, host, req->port, cbuf, length, response);
	} else {
		len = if_net_tcp_transmit(ctx, host, req->port, cbuf, length, response);
	}
	if(len != 0) {
		decode_response:
		len = http_parse_response(response, len, response);
	}
	exit_http_send:
	OS_DEBUG_EXIT();
	return len;		//unable to found +IPD prefix
}