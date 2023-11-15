#include "..\inc\if_apis.h"
#include <string.h>

//use usart4 (TX = PC10, RX = PC11), port shared with LCD (should add CS pin and mutex)
//baudrate could be configured by card

void if_ext_init(ext_context_p ctx) { 
	GPIO_InitTypeDef GPIO_InitStructure;  
	//RCC_PeriphCLKInitTypeDef RCC_InitStructure;
	//configure context
	ctx->uport = GPIOB;
	ctx->tx = GPIO_PIN_10;
	ctx->rx = GPIO_PIN_11;
	ctx->baudrate = 19200;
	//ctx->handle = USART3;
	ctx->csport = GPIOB;
	ctx->cs = GPIO_PIN_0;
	ctx->sen = GPIO_PIN_1;
	ctx->handle.Instance = USART3;
	ctx->handle.Init.BaudRate = 19200;
	//configure clock and power
	//RCC_InitStructure.PeriphClockSelection = 
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	// Ê¹ÄÜ¶Ë¿ÚÊ±ÖÓ£¬ÖØÒª£¡£¡£¡ 
	__HAL_RCC_GPIOB_CLK_ENABLE();
	//HAL_RCCEx_PeriphCLKConfig(
   	/* CS */
  	GPIO_InitStructure.Pin = ctx->cs;
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;	 		//¸´ÓÃ¿ªÂ©Êä³öÄ£Ê½
	//GPIO_InitStructure.OType = GPIO_OType_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//Êä³ö×ûÐóÆµÂÊÎª50MHz
  	HAL_GPIO_Init(ctx->csport, &GPIO_InitStructure);
  	/* SENSE  */
  	GPIO_InitStructure.Pin = ctx->sen;
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;	//¸¡¿ÕÊäÈE£Ê½
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;		//Êä³ö×ûÐóÆµÂÊÎª50MHz
  	//GPIO_Init(ctx->csport, &GPIO_InitStructure);
	//device deselect
	GPIO_SetBits(ctx->csport, ctx->cs);
}

void if_ext_sleep(ext_context_p ctx) {
	__HAL_RCC_USART3_CLK_DISABLE();
}

void if_ext_wake(ext_context_p ctx) {
	__HAL_RCC_USART3_CLK_ENABLE();
}

uint8 if_ext_send(ext_context_p ctx, uint8 * buffer, uint16 length) {
	GPIO_TypeDef prevConfig;
	GPIO_InitTypeDef GPIO_InitStructure;             
	USART_InitTypeDef USART_InitStructure;
	uint16 i;
	//check sense pin
	if((ctx->csport->IDR & ctx->sen) != 0) return -1;
	//save previous configuration
	memcpy(&prevConfig, ctx->uport, sizeof(GPIO_TypeDef));		//save previous value
	//enable clock+power
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);	// Ê¹ÄÜ¶Ë¿ÚÊ±ÖÓ£¬ÖØÒª£¡£¡£¡
	__HAL_RCC_USART3_CLK_ENABLE();
	//device select
	GPIO_ResetBits(ctx->csport, ctx->cs);
	//if_delay(100); 
   	/* UART3 Tx (PB10) */
  	GPIO_InitStructure.Pin = ctx->tx;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;	 		//¸´ÓÃ¿ªÂ©Êä³öÄ£Ê½
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;		//Êä³ö×ûÐóÆµÂÊÎª50MHz
  	HAL_GPIO_Init(ctx->uport, &GPIO_InitStructure);
  	/* UART3 Rx (PB11)  */
  	GPIO_InitStructure.Pin = ctx->rx;
  	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;	//¸¡¿ÕÊäÈE£Ê½
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;		//Êä³ö×ûÐóÆµÂÊÎª50MHz
  	HAL_GPIO_Init(ctx->uport, &GPIO_InitStructure);
	ctx->handle.Instance = USART3;
	ctx->handle.Init.BaudRate        	= ctx->baudrate; 
	ctx->handle.Init.WordLength 		= USART_WORDLENGTH_8B; //8Î»Êý¾Ý
	ctx->handle.Init.StopBits            = USART_STOPBITS_1;	 //Í£Ö¹Î»1Î»
	ctx->handle.Init.Parity              	= USART_PARITY_NONE ;	 //ÎÞ
	//USART_InitStructure.HardwareFlowControl = USART_HardwareFlowControl_None;
	ctx->handle.Init.Mode 				= USART_MODE_TX_RX;
	//USART_Init(ctx->handle, &USART_InitStructure);
	//USART_Cmd(ctx->handle, ENABLE);
	HAL_USART_Init((USART_HandleTypeDef *)ctx);
	
	//USART_SendData(ctx->handle, 0x1B); if_delay(10);
	//USART_SendData(ctx->handle, 0x3D); if_delay(10);
	//USART_SendData(ctx->handle, 0x01); if_delay(10);
	
	
	//if_ext_send(ectx, "\x1B\x3D\01", 3);		//printer online
	//if_ext_send(ectx, "\x1B\x40", 2);		//init printer
	//if_ext_send(ectx, "\x1B\x37\x7\xA0\x10", 5);		//control parameter
	for(i=0;i<length;i++) {
		//USART_SendData(ctx->handle, buffer[i]);
		if_delay(10);
	}
	//USART_SendData(ctx->handle, 0x1B); if_delay(10);
	//USART_SendData(ctx->handle, 0x40); if_delay(10);
	
	//USART_SendData(ctx->handle, 0x1B); if_delay(10);
	//USART_SendData(ctx->handle, 0x3D); if_delay(10);
	//USART_SendData(ctx->handle, 0x00); if_delay(10);
	//device deselect
	GPIO_SetBits(ctx->csport, ctx->cs);
	//restore previous configuration
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , DISABLE);	// Ê¹ÄÜ¶Ë¿ÚÊ±ÖÓ£¬ÖØÒª£¡£¡£¡
	__HAL_RCC_GPIOA_CLK_DISABLE();
	memcpy(ctx->uport, &prevConfig, sizeof(GPIO_TypeDef));		//save previous value
	return 0;
}


uint8 if_ext_sendstring(ext_context_p ctx, uint8 * str) {
	return if_ext_send(ctx, str, strlen((const char *)str));
}

TIM_HandleTypeDef g_TIM4_BaseInitStructure;

uint8 sio_build_tlv(uint8 * buffer, uint8 tag, uint8 len, uint8 * value) {
	memcpy(buffer + 2, value, len);
	buffer[0] = tag;
	buffer[1] = len;
	return len + 2;
}

void sio_init_timer(sio_context_p ctx) {
	if(ctx->state & SIO_STATE_DEVICE) {
		
	} else {
		//master clock
		g_TIM4_BaseInitStructure.Instance = TIM4;
		g_TIM4_BaseInitStructure.Init.Period = 2000;
		g_TIM4_BaseInitStructure.Init.Prescaler = 6;
		g_TIM4_BaseInitStructure.Init.ClockDivision = 10;
		g_TIM4_BaseInitStructure.Init.CounterMode = TIM_COUNTERMODE_DOWN;
		g_TIM4_BaseInitStructure.Init.RepetitionCounter = 0;
		
		HAL_TIM_Base_Init(&g_TIM4_BaseInitStructure); 
		__HAL_TIM_CLEAR_FLAG(&g_TIM4_BaseInitStructure, TIM_FLAG_UPDATE);
		__HAL_TIM_ENABLE_IT(&g_TIM4_BaseInitStructure, TIM_IT_CC1);
		NVIC_EnableIRQ(TIM4_IRQn);
		HAL_TIM_Base_Start(&g_TIM4_BaseInitStructure); 
	}
}

//host only API
uint8 sio_send_command(sio_context_p ctx, uint8 len, uint8 * cmd, uint8 * rsp) {
	uint8 i;
	uint8 res = 0;
	uint8 b;
	if(ctx->state & SIO_STATE_DEVICE) return -1;		//wrong role
	for(i=0;i<len;i++) {
		res |= sio_send_byte(ctx, cmd[i]);
	}
	if_delay(32);		//should put some timegap here
	if((res |= sio_recv_byte(ctx, &b)) != 0) return res;
	if((res |= sio_recv_byte(ctx, &len)) != 0) return res;
	for(i=0;i<len;i++) {
		if((res |= sio_recv_byte(ctx, &rsp[i])) != 0) return res;
	}
	return res;
}

//device only API
uint8 sio_send_response(sio_context_p ctx, uint8 len, uint8 * rsp) {
	uint8 i;
	uint8 res = 0;
	uint8 b;
	if((ctx->state & SIO_STATE_DEVICE) == 0) return -1;		//wrong role
	for(i=0;i<len;i++) {
		res |= sio_send_byte(ctx, rsp[i]);
	}
	return res;
}

//return length of command received
uint8 sio_wait_command(sio_context_p ctx, uint8 * tag, uint8 * buffer) {
	uint8 cmd;
	uint8 res = 0;
	uint8 i;
	uint8 len;
	//wait for command
	while(sio_recv_byte(ctx, tag) != 0);
	if(sio_recv_byte(ctx, &len) == 0) {
		res = 0;
		for(i=0;i<len;i++) {
			res |= sio_recv_byte(ctx, &buffer[i]);
			if(res != 0) break;
		}
		if(res != 0) return -1;
	}
	return len;
}

void sio_handler_loop(dev_context_p ctx) {
	uint8 cmd;
	uint8 len;
	uint8 res = 0;
	uint8 i;
	uint8 cmd_buffer[254];
	while(1) {
		while((len = sio_wait_command((sio_context_p)ctx, &cmd, cmd_buffer)) == (uint8)-1) { if_delay(10); }
		//decode command from host
		switch(cmd) {
			case DEV_CMD_ENUMERATE:
				//check for device role
				if((((sio_context_p)ctx)->state & SIO_STATE_DEVICE) == 0) break;		//wrong role
				//try sending device name, wait for specified delay if failed
				while(sio_send_response((sio_context_p)ctx, strlen((const char *)ctx->name), ctx->name) != 0) { if_delay(32); }
				break;
			case DEV_CMD_SELECT_BY_ADDR:
				if(memcmp(cmd_buffer, (uint8 *)&ctx->address, 4) == 0) {
					ctx->state |= DEV_STATE_SELECTED;
					sio_send_response((sio_context_p)ctx, sio_build_tlv(cmd_buffer, DEV_ADDRESS, 4, (uint8 *)&ctx->address), cmd_buffer);
				} else {
					//automatically deselect device if address didn't matched
					ctx->state &= ~DEV_STATE_SELECTED;
				}
				break;
			case DEV_CMD_SELECT_BY_NAME:
				if(strlen((const char *)ctx->name) != len) break;		//check for length
				if(memcmp(cmd_buffer, (uint8 *)ctx->name, len) == 0) {
					ctx->state |= DEV_STATE_SELECTED;
					sio_send_response((sio_context_p)ctx, sio_build_tlv(cmd_buffer, DEV_ADDRESS, 4, (uint8 *)&ctx->address), cmd_buffer);
				} else {
					//automatically deselect device if name didn't matched
					ctx->state &= ~DEV_STATE_SELECTED;
				}
				break;
			case DEV_CMD_SET_ADDR:
				if((ctx->state & DEV_STATE_SELECTED) == 0) break;		//check for device select state
				memcpy(&ctx->address, cmd_buffer, 4);
				//send the new device address
				sio_send_response((sio_context_p)ctx, sio_build_tlv(cmd_buffer, DEV_ADDRESS, 4, (uint8 *)&ctx->address), cmd_buffer);
				break;
			case DEV_CMD_DESELECT:
				if((ctx->state & DEV_STATE_SELECTED) == 0) break;		//check for device select state
				ctx->state &= ~DEV_STATE_SELECTED;
				break;
		}
	}
}

#define SIO_DIR_OUT			1
#define SIO_DIR_IN				0
static void sio_set_direction(void * port, uint16 pin, uint8 dir_out) {
	GPIO_InitTypeDef GPIO_InitStructure;
	//init io
	if(dir_out) {
		GPIO_InitStructure.Pin = pin;										//card ICC SW
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;					//input pulled up
		//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.Pull = GPIO_PULLUP;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	} else {
		GPIO_InitStructure.Pin = pin;										//card ICC SW
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;					//input pulled up
		//GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	}
	HAL_GPIO_Init(port, &GPIO_InitStructure);		
}

uint8 sio_send_byte(sio_context_p ctx, uint8 byte) {
	uint8 mask = 0x01;
	uint32 timeout = 1000000;
	uint8 i;
	if(ctx->state & SIO_STATE_DEVICE) {
		//device com
		sio_set_direction(ctx->port, ctx->clk, SIO_DIR_IN);
		sio_set_direction(ctx->port, ctx->io, SIO_DIR_OUT);
		//set io to low (device ready)
		GPIO_ResetBits(ctx->port, ctx->io);
		for(mask = 0x01;mask != 0; mask <<=1) {
			//wait clock until high
			while((ctx->port->IDR & ctx->clk) == 0) { timeout--; }
			//setup data during high time
			if(byte & mask) {
				GPIO_SetBits(ctx->port, ctx->io);
			} else {
				GPIO_ResetBits(ctx->port, ctx->io);
			}
			//wait clock until low
			while(ctx->port->IDR & ctx->clk) { timeout--; }
			//host should sampling bit during this falling edge
			//check for line arbitration
			sio_set_direction(ctx->port, ctx->io, SIO_DIR_IN);
			if(byte & mask) {
				if((ctx->port->IDR & ctx->io) == 0) {
					sio_set_direction(ctx->port, ctx->io, SIO_DIR_OUT);
					return -1;		//arbitration failed
				}
			}
			sio_set_direction(ctx->port, ctx->io, SIO_DIR_OUT);
		}
		//set io to high (idle mode)
		GPIO_SetBits(ctx->port, ctx->io);
	} else {
		//host com
		sio_set_direction(ctx->port, ctx->clk, SIO_DIR_OUT);
		sio_set_direction(ctx->port, ctx->io, SIO_DIR_OUT);
		//set io to low (host ready)
		//set clock to low
		GPIO_ResetBits(ctx->port, ctx->clk);
		if_delay(1);
		GPIO_ResetBits(ctx->port, ctx->io);
		for(mask = 0x01;mask != 0; mask <<=1) {
			//set clock to low
			GPIO_ResetBits(ctx->port, ctx->clk);
			//setup data during low time
			if(byte & mask) {
				GPIO_SetBits(ctx->port, ctx->io);
			} else {
				GPIO_ResetBits(ctx->port, ctx->io);
			}
			//clock currently low
			if_delay(1);
			//set clock to high
			GPIO_SetBits(ctx->port, ctx->clk);
			if_delay(1);
			sio_set_direction(ctx->port, ctx->io, SIO_DIR_IN);
			if(byte & mask) {
				if((ctx->port->IDR & ctx->io) == 0) {
					sio_set_direction(ctx->port, ctx->io, SIO_DIR_OUT);
					return -1;		//arbitration failed
				}
			}
			sio_set_direction(ctx->port, ctx->io, SIO_DIR_OUT);
		}
		//set io to high (idle mode)
		GPIO_SetBits(ctx->port, ctx->io);
		//set clock to low
		GPIO_ResetBits(ctx->port, ctx->clk);
	}
	return 0;
}

uint8 sio_recv_byte(sio_context_p ctx, uint8 * b) {
	uint32 timeout = 1000000;
	uint8 mask = 0x01;
	uint8 ret = 0;
	uint8 i;
	if(ctx->state & SIO_STATE_DEVICE) {
		sio_set_direction(ctx->port, ctx->clk, SIO_DIR_IN);
		sio_set_direction(ctx->port, ctx->io, SIO_DIR_IN);
		//wait until io become low (host ready)
		while((ctx->port->IDR & ctx->io) == 0 && timeout != 0) { timeout--; }
		//wait for clock
		for(mask = 0x01;mask != 0 && timeout != 0; mask <<=1) {
			//wait clock until high
			while((ctx->port->IDR & ctx->clk) == 0) { timeout--; }
			//sampling data during rising time
			if(ctx->port->IDR & ctx->io) ret |= mask;
			//wait clock until low
			while(ctx->port->IDR & ctx->clk) { timeout--; }
		}
	} else {
		//host com
		sio_set_direction(ctx->port, ctx->clk, SIO_DIR_OUT);
		sio_set_direction(ctx->port, ctx->io, SIO_DIR_OUT);
		//set clock to low
		GPIO_ResetBits(ctx->port, ctx->clk);
		//wait until io become low (device ready)
		while((ctx->port->IDR & ctx->io) == 0 && timeout != 0) { timeout--; }
		//start clocking
		for(mask = 0x01;mask != 0 && timeout != 0; mask <<=1) {
			//set clock to high
			GPIO_SetBits(ctx->port, ctx->clk);
			if_delay(1);
			//set clock to low
			GPIO_ResetBits(ctx->port, ctx->clk);
			//clock currently low
			if_delay(1);
			//sampling data during falling time
			//init io
			sio_set_direction(ctx->port, ctx->io, SIO_DIR_IN);
			if(ctx->port->IDR & ctx->io) ret |= mask;
			//set clock to high
			GPIO_SetBits(ctx->port, ctx->clk);
			if_delay(1);
		}
		//set clock to low
		sio_set_direction(ctx->port, ctx->io, SIO_DIR_OUT);
		GPIO_ResetBits(ctx->port, ctx->clk);
	}
	b[0] = ret;
	if(timeout == 0) return -1;		//operation timeout
	return 0;
}