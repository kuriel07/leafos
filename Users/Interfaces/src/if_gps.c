#include "defs.h"
#include "config.h"
#include <string.h>
#include "..\inc\if_apis.h"
#include "..\inc\if_net.h"
#include "..\inc\if_gps.h"
#include "if_net.h"
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif

extern uint8 simcom_gps_init(gps_context_p ctx, void * handle);

uint8 if_gps_init(gps_context_p ctx, void * handle) {
	memset(ctx, 0, sizeof(gps_context));
	if(simcom_gps_init(ctx, handle) == 0) return 0;
	return -1;
}

void if_gps_power_up(gps_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 buffer[16];
	uint8 ret = -1;
	if(ctx->init == NULL) return;		//GPS not initialized
	req.type = 0xB1;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	ret = ((uint8 *)msg->response)[0];
	os_delete_message(msg);
}

void if_gps_power_down(gps_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 buffer[16];
	uint8 ret = -1;
	if(ctx->init == NULL) return;		//GPS not initialized
	req.type = 0xB2;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	ret = ((uint8 *)msg->response)[0];
	os_delete_message(msg);
}

void if_gps_reset(gps_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 buffer[16];
	uint8 ret = -1;
	if(ctx->init == NULL) return;		//GPS not initialized
	req.type = 0xB3;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	ret = ((uint8 *)msg->response)[0];
	os_delete_message(msg);
}

uint8 if_gps_get(gps_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 buffer[16];
	uint8 ret = -1;
	if(ctx->init == NULL) return -1;		//GPS not initialized
	req.type = 0xB6;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	ret = ((uint8 *)msg->response)[0];
	os_delete_message(msg);
	return ret;
}