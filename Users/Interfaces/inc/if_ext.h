#ifndef IF_EXT__H
#define IF_EXT__H

#include "..\..\defs.h"
#include "..\..\config.h"

#define SIO_STATE_HOST				0x0000
#define SIO_STATE_DEVICE			0x8000

#define DEV_CMD_ENUMERATE				0x00
#define DEV_CMD_SELECT_BY_ADDR		0x01		//select a specified device by address
#define DEV_CMD_SELECT_BY_NAME		0x02
#define DEV_CMD_SET_ADDR					0x03		//set address at specified device
#define DEV_CMD_DESELECT					0x04

#define DEV_ACK							0x80
#define DEV_ADDRESS					0x81


#define DEV_STATE_SELECTED		0x80

typedef struct ext_context {
	USART_HandleTypeDef handle;
	GPIO_TypeDef * uport;
	uint16 tx;
	uint16 rx;
	uint32 baudrate;
	GPIO_TypeDef * csport;
	uint16 cs;
	uint16 sen;
} ext_context;

typedef struct sio_context {
	GPIO_TypeDef * port;
	uint16 io;
	uint16 clk;
	uint16 state;
} sio_context;

typedef struct dev_context {
	sio_context base;
	uint8 * name;
	uint32 address;
	uint8 state;
} dev_context;

typedef struct ext_context * ext_context_p;
typedef struct sio_context * sio_context_p;
typedef struct dev_context * dev_context_p;

void if_ext_init(ext_context_p ctx);
uint8 if_ext_send(ext_context_p ctx, uint8 * buffer, uint16 length) ;
uint8 if_ext_sendstring(ext_context_p ctx, uint8 * str);


uint8 sio_send_byte(sio_context_p ctx, uint8 byte);
uint8 sio_recv_byte(sio_context_p ctx, uint8 * b);
uint8 sio_wait_command(sio_context_p ctx, uint8 * tag, uint8 * buffer);
uint8 sio_send_response(sio_context_p ctx, uint8 len, uint8 * rsp);
uint8 sio_build_tlv(uint8 * buffer, uint8 tag, uint8 len, uint8 * value);
void sio_handler_loop(dev_context_p ctx);

#endif
