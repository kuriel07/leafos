#include "defs.h"
#include "config.h"
#include "..\inc\if_apis.h"
#include "..\inc\if_net.h"
#include "..\inc\if_ble.h"
#include "..\inc\if_gps.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include <string.h>

#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif

uint8 g_net_task_stack[14400];

uint8 gba_net_buffer[NET_BUFFER_SIZE];
uint16 gba_net_head = 0;
uint16 gba_net_tail = 0;
static net_context_p g_current_context = NULL;

net_conn * g_channels[NET_MAX_CHANNEL] = { NULL, NULL, NULL, NULL };		//global channels

void net_buffer_reset() {
	if(gba_net_head == gba_net_tail) {
		gba_net_head = 0;
		gba_net_tail = 0;
	}
}

void net_buffer_push(uint8 c) {
	gba_net_bufstream[gba_net_head++] = c;
}

uint8 net_buffer_dequeue(uint8 * c) {
	if(gba_net_tail >= gba_net_head) return -1;
	c[0] = gba_net_bufstream[gba_net_tail++];
	return 0;
}

uint16 net_buffer_length() {
	return gba_net_head;
}

uint16 net_buffer_tail() {
	return gba_net_tail;
}

net_conn * net_conn_get_client(uint16 index) {
	return g_channels[index];
}

net_conn * net_conn_create(net_context_p ctx, uint16 mode, uint32 timeout, uint16 port, uint8 * buffer, uint16 bufsz, void (* listen_callback)(void *, net_conn *)) {
	uint8 id = 0;
	net_conn * conn = NULL;
	//check if port is already opened
	if(mode & NET_LISTEN) {
		for(;id<NET_MAX_CHANNEL;id++) {
			if(g_channels[id] != NULL && g_channels[id]->port == port) return NULL;
		}
	}
	//try locating a link number
	id = 0;
	if(mode & NET_LISTEN) id = 1;		//listen not available on channel 0
	for(;id<NET_MAX_CHANNEL;id++) {
		if(g_channels[id] == NULL) break;
		
	}
	if(id == NET_MAX_CHANNEL) return conn;		//unable to reserve channel
	conn = (net_conn *)os_alloc(sizeof(net_conn));
	if(buffer == NULL) {
		buffer = (uint8 *)os_alloc(bufsz);
	}
	if(conn == NULL) return conn;
	memset(conn, 0, sizeof(net_conn));
	conn->base.mode = mode;
	conn->base.buffer = buffer;
	conn->base.bufsz = bufsz;		//maximum bufsz
	conn->base.buflen = 0;			//no data available
	conn->base.bufend = 0;
	conn->base.buf_release = os_free;
	conn->base.listen_callback = listen_callback;
	conn->id = id;
	conn->state = NET_CONN_READY;
	conn->netctx = ctx;
	conn->port = 0;
	if(mode & NET_LISTEN) conn->port = port;
	else conn->remote_port = port;
	conn->timeout = timeout;
	g_channels[id % NET_MAX_CHANNEL] = conn;		//assign conn to channel
	return conn;
}

void net_conn_close(net_conn * conn) {
	if(conn == NULL) return;
	g_channels[conn->id % NET_MAX_CHANNEL] = NULL;		//remove from channel list
	if(conn->base.buf_release != NULL) conn->base.buf_release(conn->base.buffer);
	os_free(conn);
}

uint8 if_net_ssidrec_count(void * handle) {
	OS_DEBUG_ENTRY(if_net_ssidrec_count);
	uint8 i = 0;
	uint8 counter = 0;
	net_ssidrec temp;
	for(i=0;i<4;i++) {
		if(if_net_ssidrec_read(NULL, i, &temp) == 0) { counter++; }
	}
	OS_DEBUG_EXIT();
	return counter;
}

uint8 if_net_ssidrec_read(void * handle, uint8 index, net_ssidrec * record) {
	OS_DEBUG_ENTRY(if_net_ssidrec_read);
	uint8 stat = 0;
	index = index % 4;
	if_flash_data_read(NULL, SHARD_SSIDREC_START + (index * sizeof(net_ssidrec)), (uint8 *)record, sizeof(net_ssidrec));
	if(record->magic != NET_SSIDREC_MAGIC) stat = -1;
	OS_DEBUG_EXIT();
	return stat;
}

void if_net_ssidrec_push(void * handle, uint8 * name, uint8 * username, uint8 * password) {
	OS_DEBUG_ENTRY(if_net_ssidrec_push);
	net_ssidrec rec[4];
	net_ssidrec temp;
	uint8 i,j;
	uint8 cur_idx = 0;
	//check ssidname and password length, if overflow, do not store record
	if(strlen((const char *)name) >= 29) return;
	if(strlen((const char *)username) >= 31) return;
	if(strlen((const char *)password) >= 31) return;
	//read any previous records
	if_flash_data_read(NULL, SHARD_SSIDREC_START, (uint8 *)&rec, sizeof(rec));
	//increment counter in-case oid already existed
	for(i=0;i<4;i++) {
		if(strncmp((const char *)rec[i].name, (const char *)name, 30) == 0) {
			rec[i].ordinal++;
			memcpy(rec[cur_idx].username, username, 32);
			memcpy(rec[cur_idx].password, password, 32);
			break;
		}
	}
	if(i == 4) i = 3;		//put on last record
	cur_idx = i;
	//create new record
	rec[cur_idx].magic = NET_SSIDREC_MAGIC;
	rec[cur_idx].ordinal = 1;
	memcpy(rec[cur_idx].name, name, 30);
	memcpy(rec[cur_idx].username, username, 32);
	memcpy(rec[cur_idx].password, password, 32);
	//sort record by ordinal
	for(j=0;j<3;j++) {
		if(rec[j].ordinal < rec[j+1].ordinal) {
			memcpy(&temp, &rec[j+1], sizeof(net_ssidrec));
			memcpy(&rec[j+1], &rec[j], sizeof(net_ssidrec));
			memcpy(&rec[j], &temp, sizeof(net_ssidrec));
		}			
	}
	if_flash_data_write(NULL, SHARD_SSIDREC_START, (uint8 *)&rec, sizeof(rec));
	OS_DEBUG_EXIT();
}

//callback APIs
void if_net_set_prepare_context(net_context_p ctx, void * cb_ctx) { ctx->p_ctx = cb_ctx; }
void if_net_set_prepare_callback(net_context_p ctx, void (* prepare)(void *)) { ctx->prepare_callback = prepare; }
void if_net_set_finish_context(net_context_p ctx, void * cb_ctx) { ctx->f_ctx = cb_ctx; }
void if_net_set_finish_callback(net_context_p ctx, void (* finish)(void *)) { ctx->finish_callback = finish; }
void if_net_set_exception_callback(net_context_p ctx, void  (* exception)(net_context_p, const char * )) { ctx->exception_handler = exception; }

#if SHARD_RTOS_ENABLED 

uint8 if_net_get_status(net_context_p ctx, net_status_p stats) {
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	if(ctx->init == NULL) return -1;
	req.type = 6;
	req.param1 = stats;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_net_get_ipconfig(net_context_p ctx) {
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	if(ctx->init == NULL) return -1;
	req.type = 5;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return 0;
}

uint8 if_net_disable(net_context_p ctx) {
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	OS_DEBUG_ENTRY(if_net_disable);
	req.type = 0xF9;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	OS_DEBUG_EXIT();
	return ret;
}

uint8 if_net_enable(net_context_p ctx) {
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	OS_DEBUG_ENTRY(if_net_enable);
	uint8 num_ssid;
	req.type = 0xF8;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	OS_DEBUG_EXIT();
	return ret;
}

void if_net_power_up(net_context_p ctx) {
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	req.type = 0xFA;
	if((ctx->state & IF_NET_STATE_INITIALIZED) == 0) return;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
}

uint8 if_net_power_down(net_context_p ctx) {
	uint8 ret = -1;
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	req.type = 0xFD;
	if((ctx->state & IF_NET_STATE_INITIALIZED) == 0) return ret;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

void if_net_sleep(net_context_p ctx) {
	OS_DEBUG_ENTRY(if_net_sleep);
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	if(ctx->init == NULL) goto exit_sleep; 
	--ctx->cb_depth;
	if((ctx->state & IF_NET_STATE_SLEEP) == 0 && ctx->cb_depth == 0) {
		if(ctx->finish_callback != NULL) ctx->finish_callback(ctx->f_ctx);
		ctx->state |= IF_NET_STATE_SLEEP;
		req.type = 4;
		req.param1 = (void *)1;
		msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
		os_delete_message(msg);
	}
	exit_sleep:
	OS_DEBUG_EXIT();
}

uint8 if_net_ssid_join(net_context_p ctx, uint8 * ssidname, uint8 * username, uint8 * password) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	if(ctx->init == NULL) return  -1;
	req.type = 0;
	req.param1 = ssidname;
	req.param2 = username;
	req.param3 = password;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_net_ssid_list(net_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	if(ctx->init == NULL) return 0;
	req.type = 1;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &num_ssid));
	os_delete_message(msg);
	return num_ssid;
}

uint8 if_net_try_connect(net_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 buffer[16];
	uint8 ret;
	if(ctx->init == NULL) return -1;
	req.type = 0x1E;
	req.param1 = NULL;
	req.param2 = (void *)(unsigned)sizeof(buffer);
	req.param3 = buffer;
	req.param4 = (void *)(unsigned)sizeof(buffer);
	req.param5 = buffer;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

net_conn_p if_net_tcp_open (net_context_p ctx, uint8 * host, uint16 port, uint16 mode) {
	struct net_async_params req;
	os_message * msg;
	net_conn_p ret;
	if(ctx->init == NULL) return NULL;
	req.type = 0x20;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = (void *)(unsigned)mode;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_tcp_send(net_conn_p conn, uint8 * payload, uint16 length) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	if(conn == NULL) return -1;
	req.type = 0x21;
	req.param1 = conn;
	req.param2 = payload;
	req.param3 = (void *)(unsigned)length;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_tcp_accept(net_conn_p conn) {
	OS_DEBUG_ENTRY(if_net_tcp_accept);
	uint32 i;
	uint16 ret = -1;
	uint32 timeout = conn->timeout;
	uint32 tickstart = HAL_GetTick();
	uint32 curtick;
	if(conn == NULL) goto exit_tcp_accept;
	conn->state |= NET_CONN_READY;
	while(((curtick = HAL_GetTick()) - tickstart) < timeout) { 
		if( (conn->state & NET_CONN_BUFRDY) != 0) {
			wait_next_data:
			ret = 0;
			conn->state &= ~NET_CONN_BUFRDY;		//clear ready flag
			os_wait(250);
			while(conn->state & NET_CONN_BUSY) { 
				os_wait(40); 
			}
			if((conn->state & NET_CONN_BUFRDY) != 0) goto wait_next_data;
			goto exit_tcp_accept;
		}
		os_wait(40);
	}
	exit_tcp_accept:
	OS_DEBUG_EXIT();
	return ret;
}

uint16 if_net_tcp_recv(net_conn_p conn, uint8 * response, uint16 bufsz) {
	OS_DEBUG_ENTRY(if_net_tcp_recv);
	uint16 len = 0;
	uint16 timeout = conn->timeout;
	if(conn == NULL) goto exit_tcp_recv;
	if(if_net_tcp_accept(conn) == 0) {
		len = (bufsz < conn->base.buflen)?bufsz:conn->base.buflen;
		memcpy(response, conn->base.buffer, len);
		conn->base.buflen = 0;
		conn->base.bufend = 0;
	}
	exit_tcp_recv:
	OS_DEBUG_EXIT();
	return len;
}

void if_net_tcp_close(net_conn_p conn) {
	struct net_async_params req;
	os_message * msg;
	void * ret;
	if(conn == NULL) return;
	req.type = 0x23;
	req.param1 = conn;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	//return ret;
}

void if_net_flush(net_conn_p conn) {
	struct net_async_params req;
	os_message * msg;
	void * ret;
	if(conn == NULL) return;
	req.type = 0x1F;
	req.param1 = conn;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
}

uint16 if_net_tcp_transmit (net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	if(ctx->init == NULL) return 0;
	req.type = 0x24;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

net_conn_p if_net_udp_open (net_context_p ctx, uint8 * host, uint16 port, uint16 mode) {
	struct net_async_params req;
	os_message * msg;
	net_conn_p ret;
	if(ctx->init == NULL) return NULL;
	req.type = 0x28;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = (void *)(unsigned)mode;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_udp_send (net_conn_p conn, uint8 * payload, uint16 length, uint8 addr[], uint16 port) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	if(conn == NULL) return -1;
	req.type = 0x29;
	req.param1 = conn;
	req.param2 = payload;
	req.param3 = (void *)(unsigned)length;
	req.param4 = addr;
	req.param5 = (void *)(unsigned)port;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_udp_accept(net_conn_p conn) {
	OS_DEBUG_ENTRY(if_net_udp_accept);
	uint32 i;
	uint16 ret = -1;
	uint32 timeout = conn->timeout;
	uint32 tickstart = HAL_GetTick();
	uint32 curtick;
	if(conn == NULL) goto exit_udp_accept;
	conn->state |= NET_CONN_READY;
	while(((curtick = HAL_GetTick()) - tickstart) < timeout) { 
		if( (conn->state & NET_CONN_BUFRDY) != 0) {
			ret = 0;
			conn->state &= ~NET_CONN_BUFRDY;		//clear ready flag
			goto exit_udp_accept;
		}
		os_wait(40);
	}
	exit_udp_accept:
	OS_DEBUG_EXIT();
	return ret;
}

uint16 if_net_udp_recv(net_conn_p conn, uint8 * response, uint16 bufsz) {
	OS_DEBUG_ENTRY(if_net_udp_recv);
	uint16 len = 0;
	if(conn == NULL) goto exit_udp_recv;
	if(if_net_udp_accept(conn) == 0) {
		len = (bufsz < conn->base.buflen)?bufsz:conn->base.buflen;
		memcpy(response, conn->base.buffer, len);
		conn->base.buflen = 0;
		conn->base.bufend = 0;
	}
	exit_udp_recv:
	OS_DEBUG_EXIT();
	return len;
}

void if_net_udp_close (net_conn_p conn) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	if(conn == NULL) return;
	req.type = 0x2B;
	req.param1 = conn;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
}

uint16 if_net_udp_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	if(ctx->init == NULL) return 0;
	req.type = 0x2C;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}


ssl_handle_p if_net_ssl_open(net_context_p ctx, uint8 * host, uint16 port, uint16 mode, ssl_cert_p cert, ssl_keys_p keys) {
	struct net_async_params req;
	os_message * msg;
	ssl_handle_p ret;
	req.type = 0x30;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = (void *)(unsigned)mode;
	req.param4 = cert;
	req.param5 = keys;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_ssl_send(ssl_handle_p ctx, uint8 * payload, uint16 length) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x31;
	req.param1 = ctx;
	req.param1 = payload;
	req.param2 = (void *)(unsigned)length;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_ssl_recv(ssl_handle_p ctx, uint8 * response, uint16 bufsize) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x32;
	req.param1 = ctx;
	req.param1 = response;
	req.param2 = (void *)(unsigned)bufsize;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

void if_net_ssl_close(ssl_handle_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x33;
	req.param1 = ctx;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
}

uint16 if_net_ssl_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response, ssl_cert_p cert) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x34;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	req.param6 = cert;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

void if_net_task() {
	#define TICK_LIMIT(x)		(x*5)
	OS_DEBUG_ENTRY(if_net_task);
	os_message * msg;
	net_context_p netctx = os_get_context();
	bt_context_p bctx;
	gps_context_p gctx;
	bt_handle_p bhnd;
	void * ctx;
	uint16 tick = 3;
	uint16 len;
	int32 timeout = 200;
	uint8 buffer[50];
	void * ret;
	struct net_async_params * params;
	while(1) {
		timeout = 200;
		msg = os_dequeue_message();
		if(msg != NULL) {
			ctx = msg->context;				//message context for operation (doesn't have to be netctx)
			if(ctx == NULL) goto abort_operation;
			if(msg->reqlen == 0) goto abort_operation;
			params = msg->request;
			if(netctx->state & IF_NET_STATE_ENABLED) {
				//netctx->state &= ~IF_NET_STATE_ENABLED;
				//netctx->init(netctx);
			}
			if(netctx->state & IF_NET_STATE_DISABLED) {
				//netctx->state &= ~IF_NET_STATE_DISABLED;
			}
			switch(params->type) {
				case 0:		//join
					((uint8 *)msg->response)[0] = netctx->join(ctx, params->param1, params->param2, params->param3);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 1:		//list
					((uint8 *)msg->response)[0] = netctx->list(ctx);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 4:		//sleep
					((uint8 *)msg->response)[0] = netctx->sleep(ctx);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 5:		//get ipconfig
					((uint8 *)msg->response)[0] = netctx->get_config(ctx);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 6:		//get status
					((uint8 *)msg->response)[0] = netctx->get_status(ctx, params->param1);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0xF8:		//activate
					((uint8 *)msg->response)[0] = netctx->enable(ctx);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 0xF9:		//deactivate
					((uint8 *)msg->response)[0] = netctx->disable(ctx);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 0xFA:		//power up
					((uint8 *)msg->response)[0] = netctx->power_up(ctx);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 0xFD:		//power down
					((uint8 *)msg->response)[0] = netctx->power_down(ctx);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 0x1E:		//check connection
					len = netctx->try_connect(ctx);
					if(len == 0) netctx->state |= IF_NET_STATE_CONNECTED;
					else netctx->state &= ~IF_NET_STATE_CONNECTED;
					((uint8 *)msg->response)[0] = len;
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				
				//TCP async APIs
				case 0x20:
					if(timeout != 0) os_wait(timeout);
					((net_context_p *)msg->response)[0] = NULL;
					if((ret = (void *)netctx->sock_open(ctx, params->param1, (uint16)params->param2, (uint16)params->param3 | NET_TYPE_TCP)) != 0) {
						((net_conn_p *)msg->response)[0] = ret;
					}
					msg->reslen = sizeof(net_context_p);	
					timeout = 0;
					break;
				case 0x21:
					((uint16 *)msg->response)[0] = netctx->sock_send(params->param1, params->param2, (uint16)params->param3);
					msg->reslen = sizeof(uint16);	
					timeout = 0;
					break;
				case 0x22:
					((uint16 *)msg->response)[0] = 0;
					if(netctx->sock_accept(params->param1) == 0) {
						len = netctx->sock_recv(params->param1, gba_net_buffer, (uint16)params->param3);
						if(len < (uint16)params->param3) {
							memcpy(params->param2, gba_net_buffer, len);
							((uint16 *)msg->response)[0] = len;
						}
					}
					msg->reslen = sizeof(uint16);	
					timeout = 0;
					break;
				case 0x23:
					netctx->sock_close(params->param1);
					((uint16 *)msg->response)[0] = 0;
					msg->reslen = sizeof(uint16);	
					timeout = 2000;
					break;
				case 0x24:		//tcp transmit
					if(timeout != 0) os_wait(timeout);
					((uint16 *)msg->response)[0] = netctx->send_tcp(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5);
					msg->reslen = sizeof(uint16);	
					timeout = 2000;
					break;
				
				//UDP async APIs
				case 0x28:
					if(timeout != 0) os_wait(timeout);
					((net_context_p *)msg->response)[0] = NULL;
					if((ret = netctx->sock_open(ctx, params->param1, (uint16)params->param2, (uint16)params->param3 | NET_TYPE_UDP)) != NULL) {
						((net_conn_p *)msg->response)[0] = ret;
					}
					msg->reslen = sizeof(net_context_p);	
					timeout = 0;
					break;
				case 0x29:
					((uint16 *)msg->response)[0] = netctx->sock_send(params->param1, params->param2, (uint16)params->param3);
					msg->reslen = sizeof(uint16);	
					timeout = 0;
					break;
				case 0x2A:
					((uint16 *)msg->response)[0] = 0;
					if(netctx->sock_accept(params->param1) == 0) {
						len = netctx->sock_recv(params->param1, gba_net_buffer, (uint16)params->param3);
						if(len < (uint16)params->param3) {
							memcpy(params->param2, gba_net_buffer, len);
							((uint16 *)msg->response)[0] = len;
						}
					}
					msg->reslen = sizeof(uint16);
					timeout = 0;
					break;
				case 0x2B:		//close
					netctx->sock_close(params->param1);
					((uint16 *)msg->response)[0] = 0;
					msg->reslen = sizeof(uint16);	
					timeout = 2000;
					break;
				case 0x2C:		//udp transmit
					((uint16 *)msg->response)[0] = netctx->send_udp(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5);
					msg->reslen = sizeof(uint16);
					timeout = 2000;
					break;
				
				#if SHARD_SSL_SUPPORT
				//SSL async APIs
				case 0x30:		//ssl connect
					if(timeout != 0) os_wait(timeout);
					((ssl_handle_p *)msg->response)[0] = if_ssl_open_async(ctx, params->param1, (uint16)params->param2, (uint16)params->param3, params->param4, params->param5);
					msg->reslen = sizeof(ssl_handle_p);
					timeout = 0;
					break;
				case 0x31:		//ssl send
					((uint16 *)msg->response)[0] = if_ssl_send_async(params->param1, params->param2, (uint16)params->param3);
					msg->reslen = sizeof(uint16);	
					timeout = 0;
					break;
				case 0x32:		//ssl recv
					((uint16 *)msg->response)[0] = 0;
					len = if_ssl_recv_async(params->param1, gba_net_buffer, (uint16)params->param3);
					if(len < (uint16)params->param3) {
						memcpy(params->param2, gba_net_buffer, len);
						((uint16 *)msg->response)[0] = len;
					}
					msg->reslen = sizeof(uint16);
					timeout = 0;
					break;
				case 0x33:			//ssl close
					((uint16 *)msg->response)[0] = 0;
					if_ssl_close_async(params->param1) ;
					msg->reslen = sizeof(uint16);
					timeout = 2000;
					break;
				case 0x34:		//ssl transmit
					if(timeout != 0) os_wait(timeout);
					((uint16 *)msg->response)[0] = if_ssl_transmit_async(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5, params->param6);
					msg->reslen = sizeof(uint16);
					timeout = 2000;
					break;
					#endif
				
//				BLUETOOTH APIS				
				case 0x80:			//list device
					bctx = msg->context;
					if(bctx == NULL) break;
					if(bctx->init == NULL) break;
					((uint8 *)msg->response)[0] = bctx->list(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x81:			//connect	
					bctx = msg->context;		
					if(bctx == NULL) break;
					if(bctx->init == NULL) break;
					((uint8 *)msg->response)[0] = bctx->connect(msg->context, params->param1);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x82:		//disconnect
					bctx = msg->context;		
					((uint8 *)msg->response)[0] = bctx->disconnect(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x8A:		//read
					bhnd = msg->context;
					if(bhnd == NULL) break;	
					bctx = bhnd->ctx;
					if(bctx == NULL) break;
					if(bctx->init == NULL) break;	
					((uint16 *)msg->response)[0] = bctx->read(msg->context, params->param1, (uint16)params->param2);
					msg->reslen = sizeof(uint16);
					timeout = 0;
					break;
				case 0x8B:		//write
					bhnd = msg->context;	
					if(bhnd == NULL) break;
					bctx = bhnd->ctx;	
					if(bctx == NULL) break;
					if(bctx->init == NULL) break;
					((uint8 *)msg->response)[0] = bctx->write(msg->context, params->param1, (uint16)params->param2);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x8C:
					bctx = msg->context;		
					if(bctx == NULL) break;
					if(bctx->init == NULL) break;
					((uint8 *)msg->response)[0] = if_ble_try_connect_async(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x8E:		//is_connected
					bctx = msg->context;
					if(bctx == NULL) break;
					if(bctx->init == NULL) break;
					((uint8 *)msg->response)[0] = bctx->is_connected(msg->context, params->param1);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x8F:
					bctx = msg->context;
					if(bctx == NULL) break;
					if(bctx->init == NULL) break;
					((uint8 *)msg->response)[0] = bctx->wake(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
//				END BLUETOOTH APIS	
				
//				GPS-GNSS APIS
				case 0xB0:
					gctx = msg->context;
					if(gctx == NULL) break;
					if(gctx->init == NULL) break;
					((uint8 *)msg->response)[0] = gctx->init(msg->context, params->param1);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 0xB1:			//GPS POWER_UP
					gctx = msg->context;
					if(gctx == NULL) break;
					if(gctx->init == NULL) break;
					gctx->power_up(msg->context);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 0xB2:			//GPS POWER_DOWN
					gctx = msg->context;
					if(gctx == NULL) break;
					if(gctx->init == NULL) break;
					gctx->power_down(msg->context);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 0xB3:			//GPS RESET
					gctx = msg->context;
					if(gctx == NULL) break;
					if(gctx->init == NULL) break;
					gctx->reset(msg->context);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 0xB6:			//GPS GET
					gctx = msg->context;
					if(gctx == NULL) break;
					if(gctx->init == NULL) break;
					((uint8 *)msg->response)[0] = gctx->get(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				
//				END GPS-GNSS APIS	
				
				default: 			//no API available
					((uint16 *)msg->response)[0] = -1;
					msg->reslen = sizeof(uint16);
					timeout = 0;
					break;
			}
			abort_operation:
			os_dispatch_reply(msg);
		}
		if(timeout > 200) {
			timeout -= 200;
			os_wait(200);
		} else if(timeout > 0) {
			os_wait(timeout);
			timeout = 0;
		} else 
			timeout = 0;
	}
	OS_DEBUG_EXIT();
}
#endif

void USART1_IRQHandler(void)
{
	uint8 c;
	USART_HandleTypeDef temp;
	temp.Instance = USART1;
#ifdef STM32F7
	if(g_current_context != NULL){
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&g_current_context->handle, USART_FLAG_PE)) {
			//clear PE flag
			__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)&g_current_context->handle, USART_CLEAR_PEF);
			return;
		}
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)g_current_context, USART_FLAG_RXNE) != RESET) {
			gba_net_bufstream[gba_net_head++] = HAL_USART_READ(g_current_context->handle);
		}
		__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)g_current_context, USART_CLEAR_OREF | USART_CLEAR_TCF| USART_CLEAR_CTSF);
		if_pwr_set_interrupt_source(IF_INT_NET);
	} else {
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_RXNE) != RESET) {
			c = HAL_USART_READ(temp);
		}
		__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)&temp, USART_CLEAR_OREF | USART_CLEAR_TCF| USART_CLEAR_CTSF);
	}
#endif
#ifdef STM32F4
	if(g_current_context != NULL){
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&g_current_context->handle, USART_FLAG_PE)) {
			//clear PE flag
			__HAL_USART_CLEAR_FLAG((USART_HandleTypeDef *)&g_current_context->handle, USART_FLAG_PE);
			return;
		}
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)g_current_context, USART_FLAG_RXNE) != RESET) {
			gba_net_bufstream[gba_net_head++] = HAL_USART_READ(g_current_context->handle);
		}
		__HAL_USART_CLEAR_FLAG((USART_HandleTypeDef *)g_current_context, USART_FLAG_ORE | USART_FLAG_TC);
		if_pwr_set_interrupt_source(IF_INT_NET);
	} else {
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_RXNE) != RESET) {
			c = HAL_USART_READ(temp);
		}
		__HAL_USART_CLEAR_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_ORE | USART_FLAG_TC);
	}
#endif
}

void if_net_set_timeout(net_conn_p conn, uint16 timeout) {
	conn->timeout = timeout;
}

uint8 if_net_exec_listener(net_context_p ctx) {
	net_conn_p conn = NULL;
	uint8 i=0;
	uint8 ret = -1;
	uint16 buflen = 0;
	for(i=0;i<NET_MAX_CHANNEL;i++) {
		conn = g_channels[i];
		if(conn != NULL) {
			if((conn->state & NET_CONN_BUFRDY) != 0) {
				//data exist, check available buffer length (do not clear flag until callback executed)
				while(buflen != conn->base.buflen) {
					buflen = conn->base.buflen;
					os_wait(250);			//wait for multiple data
				}
				if(conn->base.listen_callback != NULL) conn->base.listen_callback(conn->base.ctx, conn);	//exec callback
				conn->state &= ~NET_CONN_BUFRDY;	//clear flag
				conn->base.buflen = 0;					//clear buffer;
				conn->base.bufend = 0;
				ret = 0;
				break;
			}
		}
	}
	return ret;
}

void if_net_wake(net_context_p ctx) {
	OS_DEBUG_ENTRY(if_net_wake);
	if(ctx->init == NULL) goto exit_wake; 		//no driver available
	if((ctx->state & IF_NET_STATE_SLEEP) != 0 || ctx->cb_depth == 0) {
		if(ctx->prepare_callback != NULL) ctx->prepare_callback(ctx->p_ctx);
		ctx->state &= ~IF_NET_STATE_SLEEP;
	} 
	ctx->cb_depth++;
	exit_wake:
	OS_DEBUG_EXIT();
}

void if_net_set_listen_callback(net_conn_p conn, void * ctx, void (* callback)(void *, net_buffer_p)) {
	if(conn == NULL) return;
	conn->base.ctx = ctx;
	conn->base.listen_callback = callback;
}

uint8 if_net_init(net_context_p ctx) {
	memset(ctx, 0, sizeof(net_context));
	g_current_context = ctx;
	if(esp_init(ctx) == 0) return 0;
	if(simcom_init(ctx) == 0) return 0;		
	return -1;
}

void if_net_init_config(net_context_p ctx) {
	//check if driver is initialized
	if(ctx->init != NULL) {
		//net task should have at least 12K stack for wolfSSL
		os_create_task_static(ctx, if_net_task, "net", 127, (lp_void)g_net_task_stack, sizeof(g_net_task_stack));
		//os_wait(250);
	}
}

bt_device_p if_ble_dev_check(bt_context * ctx, uint8 * mac) {
	uint8 dev_count = 0;
	bt_device_p iterator = NULL;
	iterator = ctx->dev_list;
	while(iterator != NULL) {
		if(memcmp(iterator->mac, mac, 6) == 0) return iterator;
		iterator = iterator->next;
	}
	return iterator;
}

void if_ble_btrec_push(void * handle, uint8 * mac, uint8 * password) {
	OS_DEBUG_ENTRY(if_ble_btrec_push);
	net_btrec rec;
	uint8 cur_idx = 0;
	memset(&rec, 0, sizeof(rec));
	//read any previous records
	if_flash_data_read(NULL, SHARD_BTREC_START, (uint8 *)&rec, sizeof(rec));
	rec.magic = 0xEF;
	memcpy(rec.mac, mac, 6);
	if(password != NULL) memcpy(rec.password, password, 32);
	if_flash_data_write(NULL, SHARD_BTREC_START, (uint8 *)&rec, sizeof(rec));
	OS_DEBUG_EXIT();
}

uint8 if_ble_btrec_read(void * handle, net_btrec * rec) {
	if_flash_data_read(NULL, SHARD_BTREC_START, (uint8 *)rec, sizeof(net_btrec));
	if(rec->magic != 0xEF) return -1;
	return 0;
}

static uint8 if_ble_try_connect_async(bt_context * ctx) {
	OS_DEBUG_ENTRY(if_ble_try_connect_async);
	net_btrec rec;
	bt_device_p dev;
	uint8 ret = -1;
	if((ctx->state & BLE_STATE_INITIALIZED) == 0) goto exit_try_connect;
	if((ret = ctx->is_connected(ctx, ctx->slave_mac)) == 0) goto exit_try_connect;		//check if already connected 
	if(if_ble_btrec_read(NULL, &rec) == 0) {		//read for record and validate, if no record avail -> return
		if(ctx->list(ctx) != 0) {			//try scanning for any bluetooth devices
			dev = ctx->dev_list;
			while(dev != NULL) {
				if(memcmp(dev->mac, rec.mac, 6) == 0) {		//check for matching mac address
					ret = ctx->connect(ctx, dev);
					break;
				}
				dev = dev->next;
			}
		}
	}
	exit_try_connect:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 if_ble_dev_list(bt_context * ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x80;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_connect(bt_context * ctx, bt_device_p dev) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x81;
	req.param1 = dev;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_is_connected(bt_context_p ctx, uint8 * mac) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x8E;
	req.param1 = mac;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_disconnect(bt_context * ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x82;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}


uint8 if_ble_try_connect(bt_context * ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x8C;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_wake(bt_context * ctx) {
	//struct net_async_params req;
	//os_message * msg;
	uint8 ret = 0;
	//req.type = 0x8F;
	//msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	//os_delete_message(msg);
	return ret;
}

uint8 if_ble_sleep(bt_context * ctx) {
	return 0;
}

uint8 if_ble_init(bt_context * ctx, void * handle) {
	memset(ctx, 0, sizeof(bt_context));
	if(esp_ble_init(ctx, handle) == 0) return 0;
	return -1;
}

uint8 if_ble_open_static(bt_context * ctx, bt_chr_p chr, bt_handle_p handle) {
	if(chr == NULL) return -1;
	if(handle != NULL) {
		handle->ctx = ctx;
		handle->chr = chr;
	}
	return 0;
}	

bt_handle_p if_ble_open(bt_context * ctx, bt_chr_p chr) {
	OS_DEBUG_ENTRY(if_ble_open);
	bt_handle_p handle = NULL;
	if(chr == NULL) goto exit_ble_open;
	//if(chr->state & 0x01) return handle;		//check if already locked
	handle = os_alloc(sizeof(bt_handle));
	if(handle != NULL) {
		handle->ctx = ctx;
		handle->chr = chr;
		//chr->state |= 0x01;		//lock state
	}
	exit_ble_open:
	OS_DEBUG_EXIT();
	return handle;
}

bt_service * if_ble_find_service(bt_context * ctx, uint8 * uuid, uint8 uuid_len) {
	bt_service * iterator = NULL;
	if(ctx == NULL) return 0;
	if((ctx->state & BLE_STATE_CONNECTED) == 0) return 0;			//no device connected
	iterator = ctx->services;
	while(iterator != NULL) {
		if(iterator->srv_uuid_len == uuid_len) {
			if(memcmp(iterator->srv_uuid, uuid, uuid_len) == 0) return iterator;
		}
		iterator = iterator->next;
	}
	return iterator;
}

bt_chr * if_ble_find_char(bt_service * srv, uint8 * uuid, uint8 uuid_len) {
	bt_chr * iterator = NULL;
	if(srv == NULL) return 0;
	iterator = srv->chrs;
	while(iterator != NULL) {
		if(iterator->chr_uuid_len == uuid_len) {
			if(memcmp(iterator->chr_uuid, uuid, uuid_len) == 0) return iterator;
		}
		iterator = iterator->next;
	}
	return iterator;
}

uint8 if_ble_write(bt_handle_p handle, uint8 * buffer, uint16 size) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x8B;
	req.param1 = buffer;
	req.param2 = (void *)(uint32)size;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(handle, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_ble_read(bt_handle_p handle, uint8 * buffer, uint16 size) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x8A;
	req.param1 = buffer;
	req.param2 = (void *)(uint32)size;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(handle, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

void if_ble_close(bt_handle_p handle) {
	OS_DEBUG_ENTRY(if_ble_close);
	if(handle != NULL) {
		handle->chr->state = 0;		//unlock handle
		os_free(handle);
	}
	OS_DEBUG_EXIT();
}