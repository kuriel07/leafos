#include "..\..\defs.h"
#include "if_host.h"
#ifndef _OWL__H
#define _OWL__H
#define OWL_MAX_DEV			127

#define OWL_CMD_FIND			0x02						//-> TAG - L[2] - MASKBIT		-> return descriptor (automatically select device)
#define OWL_CMD_GET			0x06						//-> TAG - L[1] - ADDRESS		-> return descriptor
#define OWL_CMD_SET			0x07						//-> TAG - L[1] - ADDRESS		-> return descriptor
#define OWL_CMD_DATA		0x40						//-> TAG - L[V] - PAYLOAD		-> return data/status
#define OWL_CMD_RESET		0x7F						//-> TAG - L[0]						-> return none (automatically deselect device)

#define OWL_RES_STAT			0x80						//-> TAG - L[1] - STATUS (0 = SUCCESS)
#define OWL_RES_DESC			0x81						//-> TAG - L[V] - DESCRIPTOR
#define OWL_RES_DATA			0xC0						//-> TAG - L[V] - RESULT

#define OWD_STATE_SELECTED		0x80			

typedef struct device_descriptor * device_descriptor_p;
typedef struct owl_context * owl_context_p;
typedef struct owd_context * owd_context_p;

typedef struct device_descriptor {
	uint8 pid;							//protocol identifier
	uint8 id[2];							//device identifier
	uint8 address;					//current device address
	uint8 status;					//device status
	uint8 conf[4];					//device configuration (reserved)
	uint8 vendor[8];
	uint8 product[8];
	uint8 version;
} device_descriptor;

typedef struct owl_context {			//owl host context
	host_context base;
	uint8 devlist[OWL_MAX_DEV];
} owl_context;

void owl_init(owl_context_p ctx);
void owl_deinit(owl_context_p ctx);
uint8 owl_find(owl_context_p ctx, uint16 id, device_descriptor_p desc);
uint8 owl_get(owl_context_p ctx, uint8 address, device_descriptor_p desc);
uint8 owl_set(owl_context_p ctx, uint8 address, device_descriptor_p desc);
uint8 owl_open(owl_context_p ctx, uint16 id);
uint8 owl_transmit(owl_context_p ctx, uint8 address, uint8 * inbuf, uint8 length, uint8 * outbuf);
void owl_close(owl_context_p ctx, uint8 address);

//void owd_init(owd_context_p ctx, uint16 dev_id, uint8 * vendor, uint8 * product);
//void owd_main_loop(owd_context_p ctx);
#endif

