#include "defs.h"
#include "config.h"
#include "if_net.h"

#ifndef _IF_BLE__H
#define _IF_BLE__H

#define BLE_STATE_INITIALIZED			0x8000
#define BLE_STATE_CONNECTED			0x0001
#define BLE_STATE_OPENED					0x0002

#define BT_MODE_GAP					0x2000		//datalink layer
#define BT_MODE_L2CAP				0x1000		//datalink layer
#define BT_MODE_MCAP				0x1100		//packet layer
#define BT_MODE_RFCOMM			0x1200		//packet layer
#define BT_MODE_BNEP				0x1300		//packet layer
#define BT_MODE_SDP					0x1400		//packet layer
#define BT_MODE_SPP					0x1210		//service layer
#define BT_MODE_SAP					0x1220		//service layer
#define BT_MODE_HFP					0x1230		//service layer
#define BT_MODE_HSP					0x1240		//service layer

typedef struct bt_context bt_context;
typedef struct bt_context * bt_context_p;
typedef struct bt_device bt_device;
typedef struct bt_device * bt_device_p;
typedef struct comm_context comm_context;
typedef struct comm_context * comm_context_p;
typedef struct bt_chr bt_chr;
typedef struct bt_chr * bt_chr_p;
typedef struct bt_service bt_service;
typedef struct bt_service * bt_service_p;
typedef struct bt_handle bt_handle;
typedef struct bt_handle * bt_handle_p;

struct comm_context {
	UART_HandleTypeDef handle;
	uint32 baudrate;
	GPIO_TypeDef * port;
	uint16 tx;
	uint16 rx;
};

struct bt_device {
	uint8 mac[6];
	uint8 name[18];
	uint8 srv_id;			//service index;
	uint8 chr_id;			//characteritic index;
	struct bt_context * ctx;
	struct bt_device * next;
};

struct bt_chr {
	uint8 chr_id;
	uint8 attr;
	struct bt_service * service;
	struct bt_chr * next;
	uint8 state;
	uint8 chr_uuid_len;
	uint8 chr_uuid[0];
};

struct bt_service {
	uint8 srv_id;
	uint8 rsv;
	struct bt_chr * chrs;
	struct bt_service * next;
	uint8 state;
	uint8 srv_uuid_len;
	uint8 srv_uuid[0];
};

struct bt_handle {
	bt_context_p ctx;
	bt_chr * chr;
};

struct bt_context {
	net_context_p handle;
	//device state
	uint16 state;
	uint16 cid;
	uint16 mtu;
	uint8 slave_mac[6];
	uint8 host_mac[6];
	bt_device_p dev_list;
	bt_service_p services;		//list of all available services and characteristics on connected device
	
	//device APIs
	uint8 (* init)(bt_context_p ctx, void * handle);
	uint8 (* wake)(bt_context_p ctx);
	uint8 (* list)(bt_context_p ctx);
	uint8 (* is_connected)(bt_context_p ctx, uint8 * mac);
	uint8 (* connect)(bt_context_p ctx, bt_device_p dev);
	uint8 (* disconnect)(bt_context_p ctx);
	
	//Handle APIs
	uint8 (* write)(bt_handle_p handle, uint8 * buffer, uint16 size);
	uint16 (* read)(bt_handle_p handle, uint8 * buffer, uint16 size);
};


//BT Module Discovery APIs
uint8 if_comm_init(comm_context * ctx);
uint8 if_ble_init(bt_context * ctx, void * handle);
uint8 if_ble_get_mac(bt_context_p ctx) ;
uint8 if_ble_dev_list(bt_context * ctx);
uint8 if_ble_connect(bt_context * ctx, bt_device_p dev);
uint8 if_ble_disconnect(bt_context * ctx);
uint8 if_ble_wake(bt_context * ctx);
uint8 if_ble_sleep(bt_context * ctx);

//BT device APIs
bt_service * if_ble_find_service(bt_context * ctx, uint8 * uuid, uint8 uuid_len);
bt_chr * if_ble_find_char(bt_service * srv, uint8 * uuid, uint8 uuid_len);
bt_service_p if_ble_list_services(bt_context_p ctx);
uint8 if_ble_open_static(bt_context * ctx, bt_chr_p chr, bt_handle_p handle);
bt_handle_p if_ble_open(bt_context * ctx, bt_chr_p chr);
uint8 if_ble_write(bt_handle_p ctx, uint8 * data, uint16 size);
uint16 if_ble_read(bt_handle_p ctx, uint8 * response, uint16 size);
void if_ble_close(bt_handle_p ctx);
uint8 if_ble_try_connect(bt_context * ctx);
uint8 if_ble_is_connected(bt_context_p ctx, uint8 * mac);

//async APIs
uint16 if_ble_read_async(bt_handle_p handle, uint8 * response, uint16 size);
uint8 if_ble_write_async(bt_handle_p handle, uint8 * data, uint16 size);
uint8 if_ble_dev_list_async(bt_context * ctx);
uint8 if_ble_connect_async(bt_context * ctx, bt_device_p dev);
uint8 if_ble_disconnect_async(bt_context * ctx) ;
uint8 if_ble_try_connect_async(bt_context * ctx);
uint8 if_ble_is_connected_async(bt_context_p ctx, uint8 * mac);
uint8 if_ble_wake_async(bt_context * ctx);
#endif
