#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\defs.h"
#include "..\..\interfaces\inc\if_apis.h"

//send https request and wait for response
//request = payload in parameters ex : id=0&token=01234567
//length = payload length
#if SHARD_SSL_SUPPORT

uint16 https_send(net_context_p ctx, net_request_p req, uint8 * headers, uint8 * payload, uint16 length, uint8 * response) {
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
	uint16 len;
	//check for HTTP prefix
	if(strlen((const char *)req->uri) > sizeof(host)) return 0;		//exception, invalid host address
	ptr = net_decode_url(req->uri, &req->port, host);
	switch(req->type & 0x07) {						
		case IF_HTTP_TYPE_HEADER: cbuf[0] = 0; break;
		case IF_HTTP_TYPE_GET: 
			if(length != 0) sprintf((char *)cbuf, "GET %s?%s HTTP/1.1\r\nHost: %s\r\n\r\n", ptr, payload, host); 
			else sprintf((char *)cbuf, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", ptr, host); 
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
			sprintf((char *)cbuf, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n", ptr, host, length); 
			//calculate header length
			hdrlen = strlen((const char *)cbuf);
			//copy payload
			memcpy(cbuf + hdrlen, payload, length);
			break;
	}
	//calculate total length of request packet (2016.06.27)
	length = strlen((const char *)cbuf);
	//len = if_ssl_transmit(ctx, host, port, cbuf, length, response);
	//len = ctx->send_ssl(ctx, host, port, cbuf, length, response);
	len = if_net_ssl_transmit(ctx, host, req->port, cbuf, length, response, req->cert);
	if(len != 0) {
		decode_response:
		len = http_parse_response(response, len, response);
	}
	return len;		//unable to found +IPD prefix
}


//send https request and wait for response
uint16 https_fetch(ssl_handle_p ctx, uint8 type, uint8 * url, uint16 port, uint8 * request, uint16 length, uint8 * response) {
	//send http request
	uint8 host[210];
	uint8 lbuf[40];
	uint8 cbuf[512];
	uint16 i = 0;
	uint8 clen[10];
	uint16 plen;
	uint8 * ptr;
	uint8 * nextptr;
	uint16 dlen;
	uint16 hdrlen;
	uint16 len;
	//check for HTTP prefix
	if(strlen((const char *)url) > sizeof(host)) return 0;		//exception, invalid host address
	ptr = net_decode_url(url, &port, host);
	switch(type & 0x07) {						
		case IF_HTTP_TYPE_HEADER: cbuf[0] = 0; break;
		case IF_HTTP_TYPE_GET: 
			if(length != 0) sprintf((char *)cbuf, "GET %s?%s HTTP/1.1\r\nHost: %s\r\n\r\n", ptr, request, host); 
			else sprintf((char *)cbuf, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", ptr, host); 
			break;
		case IF_HTTP_TYPE_POST: 
			nextptr = (uint8 *)strstr((char *)ptr, (const char *)"?");
			if(nextptr != NULL) nextptr[0] = 0;		//delete host argument (GET??)
			sprintf((char *)cbuf, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n", ptr, host, length); 
			//calculate header length
			hdrlen = strlen((const char *)cbuf);
			//copy payload
			memcpy(cbuf + hdrlen, request, length);
			break;
	}
	//calculate total length of request packet (2016.06.27)
	length = strlen((const char *)cbuf);
	len = if_ssl_fetch(ctx, cbuf, length, response);
	if(len != 0) {
		decode_response:
		//compare http header
		if(memcmp(response, "HTTP/1.1 200 OK", 15) != 0) return 0;
		if((ptr = (uint8 *)strstr((char *)response, "Content-Length: ")) == NULL) return 0;
		ptr +=16;
		i = 0;
		while(ptr < (response + len)) {
			clen[i++] = *ptr;
			if(*ptr++ == '\r') break;
		}
		clen[i] = 0;
		dlen = atoi((const char *)clen);
		memcpy(response, (response + len) - dlen, dlen);
		return dlen;
	}
	return 0;		//unable to found +IPD prefix
}

#endif