#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\defs.h"
#include "..\..\interfaces\inc\if_apis.h"

uint16 raw_send(net_context_p ctx, net_request_p req, uint8 *headers, uint8 * payload, uint16 length, uint8 * response) {
	uint8 * ptr;
	uint16 len;
	uint8 host[210];
	ptr = net_decode_url(req->uri, &req->port, host);
	if(req->type & IF_TRANSPORT_UDP) {					//use preconfigured transport level
		len = if_net_udp_transmit(ctx, host, req->port, payload, length, response);
	} else {
		len = if_net_tcp_transmit(ctx, host, req->port, payload, length, response);
	}
	return len;
}

//send tcp request and wait for response
uint16 tcp_send(net_context_p ctx, uint8 * host, uint16 port, uint8 * request, uint16 len, uint8 * response) {
	uint16 ret = 0;
	uint8 i;
	for(i=0;i<3;i++) {
		if((ret = if_net_tcp_transmit(ctx, host, port, request, len, response)) != 0) break;
	}
	return ret;
}

//send udp request and wait for response
uint16 udp_send(net_context_p ctx, uint8 * host, uint16 port, uint8 * request, uint16 len, uint8 * response) {
	uint16 ret = 0;
	uint8 i;
	for(i=0;i<3;i++) {
		if((ret = if_net_udp_transmit(ctx, host, port, request, len, response)) != 0) break;
	}
	return ret;
}