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


////////////////////////////////////////////BLE APIS//////////////////////////////////////////////
/*
<api name="ble_open" owb="false" id="208" desc="open specific communication port (UART)" >
    <param name="svc_uuid" desc="service uuid" />
    <param name="chr_uuid" desc="char uuid" />
  </api>
  <api name="ble_close" owb="false" desc="close an already opened communication port">
    <param name="handle" desc="handle to com port" />
  </api>
  <api name="ble_read" owb="false" desc="read data from communication port">
    <param name="handle" desc="handle to com port" />
  </api>
  <api name="ble_send" owb="false" desc="write data to communication port, can send in sequence of bytes">
    <param name="handle" desc="handle to com port" />
    <param name="data" desc="data to send" />
  </api>
*/
//IO APIs

static void va_ble_recv(VM_DEF_ARG) {
	//<param name="handle" desc="handle to com port" />
	OS_DEBUG_ENTRY(va_ble_recv);
	bt_handle_p handle;
	uint16 len;
	if(vm_get_argument(VM_ARG, 0)->len == 0) goto exit_ble_rcv;
	if(vm_get_argument_count(VM_ARG) >= 1) {
		handle = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
		len = if_ble_read(handle, gba_net_buffer, 1024);
		vm_set_retval(vm_create_object(len, gba_net_buffer));
	}
	exit_ble_rcv:
	OS_DEBUG_EXIT();
}

static void va_ble_send(VM_DEF_ARG) {
	//<param name="handle" desc="handle to com port" />
    //<param name="data" desc="data to send" />
	OS_DEBUG_ENTRY(va_ble_send);
	bt_handle_p handle;
	vm_object * data;
	if(vm_get_argument(VM_ARG, 0)->len == 0) goto exit_ble_snd;
	if(vm_get_argument_count(VM_ARG) >= 2) {
		handle = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
		data = vm_get_argument(VM_ARG, 1);
		if_ble_write(handle, data->bytes, data->len);
	}
	exit_ble_snd:
	OS_DEBUG_EXIT();
}

static void va_ble_close(VM_DEF_ARG) {
	//<param name="handle" desc="handle to com port" />
	bt_handle_p handle;
	if(vm_get_argument(VM_ARG, 0)->len == 0) goto exit_ble_cls;
	if(vm_get_argument_count(VM_ARG) >= 1) {
		handle = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
		if_ble_close(handle);
		//((va_ble_context *)g_pVaRetval->bytes)->base.ctx = NULL;
	}
	exit_ble_cls:
	return;
}

void va_ble_open(VM_DEF_ARG) {
	//<param name="service_uuid" desc="service uuid in binary" />
    //<param name="chr uuid" desc="char uuid in binary" />  
	OS_DEBUG_ENTRY(va_ble_open);
	va_ble_context defctx;
	vm_object * svc;
	vm_object * chr;
	bt_handle_p handle;
	bt_device_p dev;
	uint8 mac[6];
	void * ptr;
	if(vm_get_argument_count(VM_ARG) < 2) goto exit_ble_open;
	svc = vm_get_argument(VM_ARG, 0);
	chr = vm_get_argument(VM_ARG, 1);
	if((g_pfrmctx->bctx->state & BLE_STATE_INITIALIZED) == 0) goto exit_ble_open;
	//check and try connect 
	if( if_ble_is_connected(g_pfrmctx->bctx, mac) != 0) goto exit_ble_open;	//
#if 0	
	{
		//try connecting to available device list (withouth scanning)
		dev = g_pfrmctx->bctx->dev_list;
		while(dev != NULL) {
			if(memcmp(dev->mac, g_pfrmctx->bctx->slave_mac, 6) == 0) {		//check for matching mac address
				break;
			}
			dev = dev->next;
		}
		//device with exact slave address available, try connecting
		if(dev == NULL) goto exit_ble_open;
		if(if_ble_connect_async(g_pfrmctx->bctx, dev) != 0) goto exit_ble_open;
	}
#endif
	//find service with associated uuid
	ptr = if_ble_find_service(g_pfrmctx->bctx, svc->bytes, svc->len);
	if(ptr == NULL) goto exit_ble_open;
	//find characteristic with associated uuid
	ptr = if_ble_find_char((bt_service_p)ptr, chr->bytes, chr->len);
	if(ptr == NULL) goto exit_ble_open;
	((va_default_context *)&defctx)->close = va_ble_close;
	((va_default_context *)&defctx)->read = va_ble_recv;
	((va_default_context *)&defctx)->write = va_ble_send;
	((va_default_context *)&defctx)->offset =  0;
	((va_default_context *)&defctx)->seek = NULL;
	((va_default_context *)&defctx)->ctx = handle;
	vm_set_retval(vm_create_object(sizeof(va_ble_context), &defctx));
	if(vm_get_retval() == VM_NULL_OBJECT) goto exit_ble_open;
	((va_ble_context *)vm_get_retval()->bytes)->base.ctx = &((va_ble_context *)vm_get_retval()->bytes)->handle;
	//memcpy(&((va_com_context *)g_pVaRetval->bytes)->handle, &usartHandle, sizeof(UART_HandleTypeDef) );
	if(if_ble_open_static(g_pfrmctx->bctx, ptr, ((va_ble_context *)vm_get_retval()->bytes)->base.ctx) != 0) {
		vm_release_object(vm_get_retval());
		vm_set_retval(VM_NULL_OBJECT);
	}
	exit_ble_open:
	OS_DEBUG_EXIT();
	return;
}