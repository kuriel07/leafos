#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\interfaces\inc\if_net.h"
#include "..\inc\netbios.h"

static void netbios_listen_callback(void * ctx, net_buffer_p netbuf) {
	uint8 wbuf[1024];
	if(netbuf->buflen != 0) {
		memcpy(wbuf, netbuf->buffer, netbuf->buflen);
	}
}


static void netbios_entry_added(void *opq, netbios_ns_entry *entry) {
	static uint8 buffer[128];
	net_context_p ctx = (net_context_p)opq;
	if(entry != NULL) {
		sprintf((char *)buffer, "NETBIOS : %s", entry->name);
		ctx->exception_handler(ctx, (const char *)buffer);
		//strncpy((char *)buffer, entry->name, NETBIOS_NAME_LENGTH);
	}
}

static void netbios_entry_removed(void *ctx, netbios_ns_entry *entry) {
	if(entry != NULL) {
		
	}
}

netbios_ns * g_netbios_svc = NULL;

uint8 netbios_translate(net_context_p ctx, uint8 * host, uint8 * ipaddr) {
    netbios_ns_entry * entry;
	uint8 ret = -1;
	if(g_netbios_svc == NULL) return ret;
    if ((entry = netbios_ns_entry_find(g_netbios_svc, (const char *)host, 0)) != NULL) {
		memcpy(ipaddr, &entry->address.b_addr, 4);
		//sprintf((char *)ipstr, "%d.%d.%d.%d", entry->address.b_addr.b1,
		//						entry->address.b_addr.b2,
		//						entry->address.b_addr.b3,
		//						entry->address.b_addr.b4);
		ret = 0;
	}
	return ret;
}

void netbios_init(void *vctx) {
	tk_context_p ctx = (tk_context_p)vctx;
	static netbios_ns_discover_callbacks dcb = { 
		0, 
		netbios_entry_added, 
		netbios_entry_removed
	};
	g_netbios_svc = netbios_ns_new(ctx->netctx);
	if(g_netbios_svc == NULL) return;
	dcb.p_opaque = ctx->netctx;
	strncpy(g_netbios_svc->name, (const char *)ctx->netctx->name, 15);
	if(g_netbios_svc != NULL) {
		netbios_ns_discover_start(g_netbios_svc,
                              22,			//60 seconds
                              &dcb);
	}
}
