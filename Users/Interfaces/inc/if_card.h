/*!\file 		icc.h
 * \brief     	iccard driver 
 * \details   	iccard driver for communicating with ISO7816
 * \author    	AGP
 * \version   	1.0
 * \date      	2016
 * \pre       	
 * use timer1 and usart2(default), pin A..0(sw, active low), A.1(rst), A.2(data, open drain, must be pulled up), A.4(clk)
 * must configure delay (systick delay), default peripheral clock at 36Mhz, use PDIV f/2 with HCLK at 72Mhz (8Mhz x 9), HPLL = 9
\verbatim	
********************************************************************
1.0
 * fixed ATR, now it finally works (2016.02.27)
 * added icc_transmit (2016.02.27)
 * wrap to icc_context (2016.02.27)
 * added ATR decode and process (2016.02.27)
 * added icc_send_pps (pps mechanism, 2016.02.28)
 * fixed PPS mechanism, icc_get_atr (2016.02.29)
 * fixed Le mechanisme when requesting 256 bytes of data (2016.03.16)
 * added: implementation of T=1 protocol (2016.03.25)
 * fixed: T=1 and T=0 mechanism, ATR parsing (2016.03.26)
 * fixed: bit sequencer N(R) and N(S) for T=1 (2016.03.26)
 * fixed: ATR decoding (when card already inserted during power on), IFS handshake for T=1 (2016.04.05)
 * fixed: infinite loop during icc_recv_block for T=1, timer couldn't be start (2016.04.10)
 * fixed: n(s) when receiving R_BLOCK, icc_send_block do not increment ns, ns only incremented after successful block processing (2016.05.20)
 * added support for STM32F7 (2018.07.29)
********************************************************************
\endverbatim
 */
#ifndef IF_CARD__H
#define IF_CARD__H

#include "..\..\config.h"
#include "..\..\defs.h"
//#include "usart.h"

 
#define ICC_MODE_RECEIVE		1
#define ICC_MODE_TRANSMIT	0

#define ICC_TIMEOUT_START		1
#define ICC_TIMEOUT_STOP		0

#define ICC_USART_ID		2
#if (ICC_USART_ID == 2)
#define ICC_USART		((USART_TypeDef *) (APB1PERIPH_BASE + 0x4400))
#define ICC_RST			((uint16_t)0x0002)
#define ICC_CLK			((uint16_t)0x0010)
#define ICC_IO				((uint16_t)0x0004)
#define ICC_SW				((uint16_t)0x0001)
#define ICC_PORT			((GPIO_TypeDef *) GPIOA_BASE)
#define ICC_PERIPH		RCC_APB1Periph_USART2
#define ICC_PORT_PERIPH	((uint32_t)0x00000004)
#endif
#if (ICC_USART_ID == 3)
#define ICC_USART		((USART_TypeDef *) (APB1PERIPH_BASE + 0x4800))
#define ICC_RST			((uint16_t)0x0200)
#define ICC_CLK			((uint16_t)0x1000)
#define ICC_IO				((uint16_t)0x0400)
#define ICC_SW				 ((uint16_t)0x0100)
#define ICC_PORT			((GPIO_TypeDef *) GPIOB_BASE)
#define ICC_PERIPH		RCC_APB1Periph_USART3
#define ICC_PORT_PERIPH	((uint32_t)0x00000008)
#endif

#define ICC_PROTOCOL_T0			0
#define ICC_PROTOCOL_T1			1

#define ICC_PCB_IBLOCK				0x00
#define ICC_PCB_RBLOCK				0x80
#define ICC_PCB_SBLOCK				0xC0
#define ICC_IPCB_MOREBIT			0x20
#define ICC_SPCB_RESPONSE				0x20
#define ICC_SPCB_REQUEST				0x00
#define ICC_SPCB_RESYNCH				0x00
#define ICC_SPCB_IFS							0x01
#define ICC_SPCB_ABORT					0x02
#define ICC_SPCB_WTX						0x03



typedef struct icc_context {
	SMARTCARD_HandleTypeDef handle;
	GPIO_TypeDef * port;
	uint16 rst;
	uint16 clk;
	uint16 io;
	uint16 sw;
	uint32 baudrate;
	uint8 fidi;
	uint8 protocol;
	uint8 ifs;					//max information frame length (card-terminal)
	uint8 edc;
	uint8 ns;						//sequence number for sending
	uint16 wtx;
	uint16 guardtime;
	uint8 atrlen;
	uint8 atr[36];
} icc_context;

typedef struct icc_context scard_context;
typedef struct icc_context * scard_context_p;

void icc_init(icc_context * ctx) ;
uint8 icc_active(icc_context * ctx);
void icc_reset_card(icc_context * ctx, uint8 mode) ;
void icc_config(icc_context * ctx, uint8 fidi, uint8 io);
uint16 icc_get_atr(icc_context * ctx, uint8 * buffer, uint32 timeOut);
void icc_sendbyte(icc_context * ctx, uint16 data);
void icc_sendbytes(icc_context * ctx, uint8 * data, uint16 length);
uint16 icc_transmit(icc_context * ctx, BYTE * c_apdu, uint16 length, BYTE * r_apdu);

//scard apis definition
void if_card_init(void * display, scard_context * ctx);
uint8 if_card_list(scard_context * ctx);
uint8 if_card_connect(scard_context * ctx, BYTE mode, BYTE * atr);
uint16 if_card_send(scard_context * ctx, BYTE * c_apdu, uint8 length, BYTE * r_apdu);
uint8 if_card_disconnect(scard_context * ctx, BYTE mode);
uint8 if_card_sleep(scard_context * ctx);
void if_card_wake(scard_context * ctx);

#endif
