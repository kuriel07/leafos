#include "defs.h"
#include "config.h"
#ifndef _IF_HOST__H
#define _IF_HOST__H

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE END PFP */
#define HS_WAIT_SOF			0x0000
#define HS_BIT_0					0x0000
#define HS_BIT_1					0x0001
#define HS_BIT_2					0x0002
#define HS_BIT_3					0x0003
#define HS_BIT_4					0x0004
#define HS_BIT_5					0x0005
#define HS_BIT_6					0x0006
#define HS_BIT_7					0x0007
#define HS_RECEIVE				0x0100
#define HS_TRANSMIT			0x0200
#define HS_LEN					0x0400

#define HS_SPACE				0x0E00			//wait for specified
#define HS_STALL				0x0D00
#define HS_INIT					0xFF00

typedef struct host_context * host_context_p;
typedef struct host_context {
	GPIO_TypeDef * base;
	uint16 sio;
	uint16 clk;
} host_context;

//machine dependent code
void host_send(host_context_p ctx, unsigned char * buffer, unsigned length) ;
unsigned char host_recv(host_context_p ctx, unsigned char * buffer) ;
void host_init(host_context_p ctx);
void host_deinit(host_context_p ctx);

#endif
