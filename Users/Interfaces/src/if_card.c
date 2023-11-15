#include "defs.h"
#include "config.h"
#include "..\inc\if_apis.h"
#include <string.h>
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif

void icc_sendbyte(icc_context * ctx, uint16 data)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8 c;
	uint32 cr1, cr2, cr3;
	while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx->handle.Instance, USART_FLAG_TXE) == RESET) {
		if(icc_active(ctx)) return;
	}
	
	HAL_USART_WRITE(ctx->handle, (data & (uint16_t)0x0FF));
}

void icc_sendbytes(icc_context * ctx, uint8 * data, uint16 length)
{
	uint16 i;
	uint16 len = length;
#ifdef STM32F7
	HAL_SMARTCARD_Transmit((SMARTCARD_HandleTypeDef *)ctx, data, len, 400);
#endif
#ifdef STM32F4
	for (i=0; i<len; i++)
	{
		icc_sendbyte(ctx, data[i]);
	}	
#endif
}

uint8 g_bIccReady = 0;
uint8 g_bIccTimeout = 0;
uint8 g_bIccBuff[5];
CONST uint16 g_bDi[] = { 1, 1, 2, 4, 8, 16, 32, 1, 12, 20, 1, 1, 1, 1, 1, 1 };
CONST uint16 g_bFi[] = { 372, 372, 558, 744, 1116, 1488, 1860, 3720, 372, 512, 768, 1024, 1536, 2048, 4096, 8192 };

uint8 g_bPduBuff[32];
uint8 g_bPduHead = 0;
uint8 g_bPduTail = 0;

void USART2_IRQHandler(void) {
	uint8 c;
	USART_HandleTypeDef temp;
	temp.Instance = USART2;
#ifdef STM32F7
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_PE)) {
		__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)&temp, USART_CLEAR_PEF);
		return;
	}
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_RXNE)) { 	
		g_bPduBuff[g_bPduHead++] = c = HAL_USART_READ(temp);
		g_bPduHead &= 0x1F;
	} 
	__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)&temp, USART_CLEAR_OREF | USART_CLEAR_TCF| USART_CLEAR_CTSF);
#endif
#ifdef STM32F4
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_PE)) {
		__HAL_USART_CLEAR_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_PE);
		return;
	}
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_RXNE)) { 	
		g_bPduBuff[g_bPduHead++] = c = HAL_USART_READ(temp);
		g_bPduHead &= 0x1F;
	} 
	__HAL_USART_CLEAR_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_ORE | USART_FLAG_TC );
#endif
	if_pwr_set_interrupt_source(IF_INT_WAKE | IF_INT_CARD);
}

uint8 icc_receive_byte(icc_context * ctx) {
	uint8 c = g_bPduBuff[g_bPduTail++];
	g_bPduTail &= 0x1F;
	return c;
}

uint32 g_wTimerCounter = 0;
TIM_HandleTypeDef  g_TIM1_BaseInitStructure; 
void TIM1_CC_IRQHandler(void) 
{
	if(g_wTimerCounter != 0) g_wTimerCounter--;
	if(g_wTimerCounter == 0) {
		g_bIccTimeout = 1;
	} 
	__HAL_TIM_CLEAR_FLAG(&g_TIM1_BaseInitStructure, TIM_FLAG_CC1);
}

void icc_reset_card(icc_context * ctx, uint8 mode) {
	USART_InitTypeDef USART_InitStructure;
	//if(mode) {	
	HAL_GPIO_WritePin(ctx->port, ctx->rst, GPIO_PIN_SET);
	icc_config(ctx, 0x11, ICC_MODE_RECEIVE);
	if_delay(60);
	HAL_GPIO_WritePin(ctx->port, ctx->rst, GPIO_PIN_RESET);
	if_delay(80);
	HAL_GPIO_WritePin(ctx->port, ctx->rst, GPIO_PIN_SET);
}

uint8 icc_active(icc_context * ctx) {
	if(ctx == NULL) return 1;
	//older board use normally opened
	return (ctx->port->IDR & ctx->sw) != 0;
	//newer board normally closed
	return (ctx->port->IDR & ctx->sw) == 0;
}

void EXTI0_IRQHandler() {
	//card inserted interrupt
  	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_0);
		if_pwr_set_interrupt_source(IF_INT_WAKE | IF_INT_CARD);
	}
}



uint8 if_card_sleep(scard_context_p ctx) {
	OS_DEBUG_ENTRY(if_card_sleep);
	__HAL_RCC_USART2_CLK_DISABLE();
	__HAL_RCC_TIM1_CLK_DISABLE();
	OS_DEBUG_EXIT();
	return 0;
}

void if_card_wake(scard_context_p ctx) {
	OS_DEBUG_ENTRY(if_card_wake);
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_TIM1_CLK_ENABLE();
	OS_DEBUG_EXIT();
}

void icc_init(icc_context * ctx) {
	//TIM_TimeBaseInitTypeDef TIM_BaseInitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
#ifdef STM32F7
	__HAL_RCC_USART2_CONFIG(RCC_USART2CLKSOURCE_PCLK1);
#endif
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	
	//icc clock
	GPIO_InitStructure.Pin = ICC_CLK;	//CK
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;			//peripheral (alternate function push pull)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;	
	GPIO_InitStructure.Alternate = GPIO_AF7_USART2;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);		
	//icc data
	GPIO_InitStructure.Pin = ICC_IO;	//TX
  	//GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;			//peripheral (alternate function push pull)	
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;			//peripheral (alternate function push pull)	
	GPIO_InitStructure.Pull = GPIO_PULLUP;	
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;	
	GPIO_InitStructure.Alternate = GPIO_AF7_USART2;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);			
	//icc reset
	GPIO_InitStructure.Pin = ICC_RST;										//card ICC RST
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		//peripheral (output push pull)
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;		
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);					
	HAL_GPIO_WritePin(GPIOA, ICC_RST, GPIO_PIN_SET);	
	//icc switch
	GPIO_InitStructure.Pin = ICC_SW;										//card ICC SW
  	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;		//GPIO_MODE_INPUT;					//input pulled up
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);	
	NVIC_EnableIRQ(EXTI0_IRQn);
	//default context
	ctx->port = ICC_PORT;
	ctx->io = ICC_IO;
	ctx->rst = ICC_RST;
	ctx->clk = ICC_CLK;
	ctx->sw = ICC_SW;
	ctx->handle.Instance = USART2;
	ctx->fidi = 0x11;										//default PPS
	ctx->guardtime = 12;									//default guardtime
	ctx->ns = 0;
	
	icc_config(ctx, 0x11, ICC_MODE_RECEIVE);
}

static void icc_timer_start(uint8 mode, uint16 wtx) {
	if(mode == ICC_TIMEOUT_START) {
		g_wTimerCounter = wtx * 1000;
		g_bIccTimeout = 0;
		g_TIM1_BaseInitStructure.Instance = TIM1;
		g_TIM1_BaseInitStructure.Init.Period = 50000;
		g_TIM1_BaseInitStructure.Init.Prescaler = 1000;
		g_TIM1_BaseInitStructure.Init.ClockDivision = 100;
		g_TIM1_BaseInitStructure.Init.CounterMode = TIM_COUNTERMODE_DOWN;
		g_TIM1_BaseInitStructure.Init.RepetitionCounter = 0;
		
		__HAL_TIM_CLEAR_FLAG(&g_TIM1_BaseInitStructure, TIM_FLAG_UPDATE);
		__HAL_TIM_ENABLE_IT(&g_TIM1_BaseInitStructure, TIM_IT_CC1);
		NVIC_EnableIRQ(TIM1_CC_IRQn);
		__HAL_TIM_ENABLE(&g_TIM1_BaseInitStructure);
	} else {
		__HAL_TIM_DISABLE(&g_TIM1_BaseInitStructure);    
		NVIC_DisableIRQ(TIM1_CC_IRQn);
	}
}

static void icc_timeout(icc_context * ctx, uint8 start) {
	if(start == ICC_TIMEOUT_START) {
		g_bIccReady = 0;
#ifdef STM32F7
		HAL_SMARTCARDEx_TimeOut_Config((SMARTCARD_HandleTypeDef *)ctx, ctx->wtx * ctx->baudrate);
		HAL_SMARTCARDEx_EnableReceiverTimeOut((SMARTCARD_HandleTypeDef *)ctx);
#endif
		__HAL_USART_ENABLE_IT((SMARTCARD_HandleTypeDef *)ctx, USART_IT_RXNE);
		__HAL_USART_ENABLE_IT((SMARTCARD_HandleTypeDef *)ctx, USART_IT_PE);
		NVIC_EnableIRQ(USART2_IRQn);
	} else {
		NVIC_DisableIRQ(USART2_IRQn);
#ifdef STM32F7
		HAL_SMARTCARDEx_DisableReceiverTimeOut((SMARTCARD_HandleTypeDef *)ctx);
#endif
	}
}

static int8 icc_send_pps(icc_context * ctx, uint8 fidi, uint8 protocol) {
	USART_InitTypeDef USART_InitStructure;
	uint8 i, b, ret = 0;
	uint8 pps[4] = { 0xFF, 0x10, 0, 0 };
    uint8 ppr[4];
	uint32 tickstart = HAL_GetTick();
	pps[1] = 0x10 | (protocol & 0x0F);
	pps[2] = fidi;
	pps[3] = (pps[0] ^ pps[1] ^ pps[2]);
	g_bPduTail = g_bPduHead = 0;
	icc_sendbytes(ctx, pps, 4);
	icc_timeout(ctx, ICC_TIMEOUT_START);
	start_timeout:
	for(i = 0;i<4;i++) {
		tickstart = HAL_GetTick();
		while(g_bPduTail == g_bPduHead && HAL_GetTick() < (tickstart + 100)) {
			if(icc_active(ctx)) return -1;		//card removed
		}
		if(HAL_GetTick() >= (tickstart + 100)) break;
		b = icc_receive_byte(ctx);
		//if(b == 0xFF) i = 0;			//check for initial PPSS
		ppr[i] = b;
	}
	icc_config(ctx, fidi, ICC_MODE_RECEIVE);
	if((ret = memcmp(pps, ppr , 4)) != 0) goto exit_pps;
	exit_pps:
	icc_timer_start(ICC_TIMEOUT_STOP, ctx->wtx);
	if(i != 4) return -1;
	if(ret != 0) return 0;
	else return protocol;
}

//receive block for T=1
static uint16 icc_recv_block(icc_context * ctx, uint8 * buffer, uint16 maxlen) {
	uint8 specific = 0;
	uint8 state = 0;
	uint8 len = 0, i = 0, b;
	uint8 chksum = 0;
	uint16 ret = 0;
	uint32 tickstart = HAL_GetTick();
	g_bIccReady = 0;
	g_bIccTimeout = 0;
	g_bPduHead = 0;
	g_bPduTail = 0;
	while(HAL_GetTick() < (tickstart + 8000)) {
		if(icc_active(ctx)) return -1;		//card removed
		if(g_bPduTail != g_bPduHead) {
			b = icc_receive_byte(ctx);
			g_bIccReady = 0;
			switch(state) {
				case 0: buffer[0] = b; state++; break;		//NAD
				case 1: buffer[1] = b; state++; break;		//PCB
				case 2: 
					len = buffer[2] = b; 
					state++; 
					i=0; 
					if(len == 0) state = 4;		//no data (usually R-BLOCK or S-BLOCK)
					break;		//LEN
				case 3:
					if(maxlen > (3 + i)) buffer[3 + i] = b;
					i++;
					if(i == len) state++;
					break;
				case 4:
					if(maxlen > (3 + i)) buffer[3 + i] = b;			//CHKSUM
					if(b == chksum) ret = (4 + i);
					goto recv_block_exit;
			} 
			chksum ^= b;
		}
	}
	while(HAL_GetTick() >= (tickstart + 8000)) return -1;
	recv_block_exit:
	return ret;
}

//send block for T=1
static uint8 icc_send_block(icc_context * ctx, uint8 pcb, uint8 * buffer, uint8 length) {
	uint8 packet[262];
	uint16 len = 0;
	uint8 chksum = 0;
	uint8 nad = 0, i;
	packet[len++] = nad;
	switch(pcb & 0xC0) {
		case ICC_PCB_IBLOCK:
			pcb = pcb | ((ctx->ns & 0x01) << 6);
			chksum = (nad ^ pcb ^ length);
			packet[len++] = pcb;
			packet[len++] = length;
			for(i=0;i<length;i++) {
				chksum ^= buffer[i];
				packet[len++] = buffer[i];
			}
			break;
		case ICC_PCB_RBLOCK:
			length = 0;
			pcb = pcb;
			chksum = (nad ^ pcb ^ length);
			packet[len++] = pcb;
			packet[len++] = length;
			break;
		case ICC_PCB_SBLOCK:
			chksum = (nad ^ pcb ^ length);
			packet[len++] = pcb;
			packet[len++] = length;
			for(i=0;i<length;i++) {
				chksum ^= buffer[i];
				packet[len++] = buffer[i];
			}
			break;
	}	
	packet[len++] = chksum;			//EDC (LRC)
	icc_sendbytes(ctx, packet, len);
	return 0;
}

//S-Block command decoder for T=1
uint8 icc_sblock_decode(icc_context * ctx, uint8 * buffer, uint8 length) {
	uint8 ret = 0;
	switch(buffer[1] & 0x3F) {
		case ICC_SPCB_RESYNCH: ctx->ns = 0; break;
		case ICC_SPCB_IFS: ctx->ifs = buffer[3]; break;
		case ICC_SPCB_ABORT: ret = -1; break;
		case ICC_SPCB_WTX: break;
	}
	//send response S-Block
	icc_send_block(ctx, buffer[1] | ICC_SPCB_RESPONSE, buffer, length); 
	return 0;
}

//send information block for T=1
uint16 icc_send_iblock(icc_context * ctx, uint8 * bufin, uint16 length, uint8 * bufout) {
	uint16 i;
	uint16 tlen;
	uint8 retry = 0;
	uint8 pcb;
	uint8 tbuf[258];
	uint8 blen = ctx->ifs;
	for(i=0; i<length; i+=ctx->ifs) {
		if((i + blen) > length) blen = length - i;
		restart_transmission:
		if(retry == 3) return 0;
		//send I-Block
		pcb = ICC_PCB_IBLOCK;
		if((length - i) > ctx->ifs) pcb |= ICC_IPCB_MOREBIT;
		icc_send_block(ctx, pcb, bufin + i, blen); 
		//wait R-Block
		tlen = icc_recv_block(ctx, tbuf, sizeof(tbuf));
		if(tlen == (uint16)-1) return -1;
		switch(tbuf[1] & 0xC0) {
			case ICC_PCB_SBLOCK: 
				if(icc_sblock_decode(ctx, tbuf, tlen) != 0) return 0; 		//abort transaction
				break;
			case ICC_PCB_RBLOCK:
				if(((tbuf[1] & 0x10) >> 4) == (ctx->ns & 0x01)) { retry++; goto restart_transmission; } //invalid N(R)
				if((tbuf[1] & 0x2F) != 0) { retry++; goto restart_transmission; }
				break;
			case (ICC_PCB_IBLOCK | 0x40):
			case ICC_PCB_IBLOCK: 
				ctx->ns++;			//increment n(s) on successful block sending
				goto start_receiving;
		}
		retry = 0;
	}
	//wait I-Block
	start_receiving:
	i = 0;
	next_chain:
	if(tlen == 0) return 0;		//recv block timeout (no response)
	pcb = tbuf[1];
	switch(tbuf[1] & 0xC0) {
		case ICC_PCB_SBLOCK: 
			if(icc_sblock_decode(ctx, tbuf, tlen) != 0) return 0; 		//abort transaction
			break;
		case ICC_PCB_RBLOCK: return 0;		//invalid R-Block
		case (ICC_PCB_IBLOCK | 0x40):
		case ICC_PCB_IBLOCK: 
			ctx->ns++;				//increment n(s) on successful block sending
			memcpy(bufout + i, tbuf + 3, tbuf[2]);
			i += tbuf[2];
			//should check LRC here (not implemented, assuming LRC always correct)
			if(tbuf[1] & ICC_IPCB_MOREBIT) {
				//send R-Block
				icc_send_block(ctx, ICC_PCB_RBLOCK | ((pcb & 0x40) >> 2), tbuf, 0); 
				//receiving next chain
				tlen = icc_recv_block(ctx, tbuf, sizeof(tbuf));
				if(tlen == (uint16)-1) return -1;
				goto next_chain;
			}
			break;
	}
	return i;
}

//send supervisory block for T=1
uint8 icc_send_sblock(icc_context * ctx, uint8 mode, uint8 * buffer, uint8 length) {
	uint16 tlen;
	//send I-Block
	icc_send_block(ctx, ICC_PCB_SBLOCK | (mode & 0x3F), buffer, length); 
	//wait S-Block (response)
	tlen = icc_recv_block(ctx, buffer, length);
	if(tlen == (uint16)-1) return -1;
	if((buffer[1] & 0xC0) == ICC_PCB_SBLOCK) {
		if((buffer[1] & 0x1F) != mode) return -1;
	}
	return 0;
}

uint16 icc_get_atr(icc_context * ctx, uint8 * buffer, uint32 timeOut) {
		uint8 i=0;
		uint8 index;
		uint8 b;
		uint8 ifbyte;
		uint8 hisbyte;
		uint8 ifmask;
		uint8 ifindex = 0;
		uint8 t15exist = 0;
		uint8 checkAtr = 1;
		uint8 tryPPS = 0x11;
		uint32 iccTimeOut = timeOut;
		uint8 specific = 0;
		uint8 rstcnt = 0;
		int8 protocol;
		uint32 tickstart = HAL_GetTick();
		reset_card:
		i=0;
		checkAtr = 1;
		iccTimeOut = timeOut;
		icc_reset_card(ctx, 0);
		icc_config(ctx, 0x11, ICC_MODE_RECEIVE);
		g_bPduTail = g_bPduHead = 0;
		icc_timeout(ctx, ICC_TIMEOUT_START);
		g_bIccReady = 0;
		tickstart = HAL_GetTick();
		while(iccTimeOut && checkAtr) {
			restart_wait:
			while(g_bPduTail == g_bPduHead && HAL_GetTick() < (tickstart + timeOut)) { 
				if_delay(1);
				if(icc_active(ctx)) return -1;
			}
			if(HAL_GetTick() >= (tickstart + timeOut)) iccTimeOut = 0;
			if(iccTimeOut) {
			//if(g_bIccTimeout) {
				b = icc_receive_byte(ctx);
				g_bIccReady = 0;
				switch(i) {
					case 0:		//TS (initial character) should be 0x3B or 0x3F
						if(b != 0x3B && b != 0x3F) goto restart_wait;
						break;
					case 1:		//T0 (format character)
						ifbyte = (b & 0xF0);
						hisbyte = b & 0x0F;
						ifmask = 0x10;
						break;
					default:
						if(ifmask != 0) {
							//for(ifmask=0x10;ifmask!=0x0;ifmask<<=1) {
							while(ifmask !=0 && (ifmask & ifbyte) == 0) ifmask <<= 1;
							switch(ifmask & ifbyte) {
								case 0x10:
									if((ifbyte & ifmask) == 0) { ifmask<<=1; goto check_tb; }
									if(ifindex == 0) {		//check for Fi/Di (TA1)
										//for PPS exchange
										tryPPS = b;
									}
									if(ifindex == 1) {		//specific mode (TA2)
										//tryPPS = b;
										specific = 1;
									}
									if((ctx->protocol & 0x0F) == ICC_PROTOCOL_T1) {
										//set maximum frame size per-transfer
										ctx->ifs = b;
									}
									ifmask<<=1;
									break;
								case 0x20:
									check_tb:
									if((ifbyte & ifmask) == 0) { ifmask<<=1; goto check_tc; }
									ifmask<<=1;
									break;
								case 0x40:
									check_tc:
									if((ifbyte & ifmask) == 0) { ifmask<<=1; goto check_td; }
									if(ifindex == 0) {		//extra guardtime (N)
										ctx->guardtime = 12 + b;
									}
									if((ctx->protocol & 0x0F) == ICC_PROTOCOL_T1) {
										//set error detection code
										ctx->edc = b;
									}
									ifmask<<=1;
									break;
								case 0x80:
									check_td:
									if((ifbyte & ifmask) == 0) { ifbyte = 0; ifmask<<=1; goto check_historical; }
									ifbyte = b & 0xF0;
									//ifmask = 0x01;
									if(ifindex == 0) {
										if((b & 0x0F) == 0x01) {
											ctx->protocol = ICC_PROTOCOL_T1;	//prefer T=1
										}
										if((b & 0x0F) == 0x0F) t15exist = 1;
									}
									ifmask = 0x10;
									ifindex++;
									break;
								default: break;
							}
							//}
						} else if(hisbyte != 0) {
							check_historical:
							hisbyte--;
							if(t15exist == 0 && hisbyte == 0) goto finish_atr;
						} else {
							//checksum (TCK)
							finish_atr:
							checkAtr = 0;
						}
						break;
				}
				ctx->atr[i++] = b;
			}
		}
		icc_timeout(ctx, ICC_TIMEOUT_STOP);
		ctx->wtx = 60;			//default wtx for T=0
		//Set the Guard Time
#ifdef USE_HAL_DRIVER
		ctx->handle.Init.GuardTime = ctx->guardtime;
		ctx->handle.Init.GuardTime = 0;
		MODIFY_REG(ctx->handle.Instance->GTPR, USART_GTPR_GT, ((ctx->handle.Init.GuardTime)<<8));
#else
		USART_SetGuardTime(ctx->handle, ctx->guardtime);
#endif
		//try changing PPS
		if(rstcnt == 3) return -1; //counter exceeded
		if(tryPPS != 0x11 || (ctx->protocol & 0x0F) != ICC_PROTOCOL_T0) {
			if_delay(20);
			if((protocol = icc_send_pps(ctx, tryPPS, ctx->protocol)) == -1) {		//check for error
				if(icc_active(ctx)) return -1;		//check if card still inserted
				rstcnt++;
				if_delay(100);
				goto reset_card;
			}
			//switchback to T=0
			ctx->protocol = (uint8)protocol;	//use default protocol
		}
		if(iccTimeOut == 0) {
			iccTimeOut = timeOut;		//reset timeout
			rstcnt++;
			if_delay(100);
			goto reset_card;
		}
		
		//set reader configuration
		if(buffer != NULL) memcpy(buffer, ctx->atr, i);
		ctx->atrlen = i;
		//intialize protocol T=1 if selected
		if((ctx->protocol & 0x0F) == ICC_PROTOCOL_T1) {
			//configure T=1 protocol
			ctx->wtx = 60;
			//change maximum frame size
			b = 0x80;
			if(icc_send_sblock(ctx, ICC_SPCB_IFS, (uint8 *)&b, 1) == 0) ctx->ifs = 0x80;
			else return -1;
		}
		return i;
}

void icc_config(icc_context * ctx, uint8 fidi, uint8 io)
{
	USART_InitTypeDef USART_InitStructure;
	ctx->baudrate = (HAL_RCC_GetPCLK1Freq() / 10) / (g_bFi[fidi >> 4] / g_bDi[fidi & 0x0F])  ;
	ctx->fidi = fidi;	
	ctx->handle.Init.BaudRate       	= ctx->baudrate;
	ctx->handle.Init.WordLength 		= USART_WORDLENGTH_9B;
	ctx->handle.Init.StopBits           = USART_STOPBITS_1_5;	 
	ctx->handle.Init.Parity             = USART_PARITY_EVEN ;	 
	ctx->handle.Init.Mode 				= SMARTCARD_MODE_TX_RX;
	ctx->handle.Init.CLKPolarity 		= USART_POLARITY_LOW;
	ctx->handle.Init.CLKPhase 			= USART_PHASE_1EDGE;
	ctx->handle.Init.CLKLastBit  		= USART_LASTBIT_DISABLE;
#ifdef STM32F7 
	ctx->handle.Init.NACKEnable 		= SMARTCARD_NACK_DISABLE;
	ctx->handle.Init.TimeOutEnable 		= SMARTCARD_TIMEOUT_DISABLE;
	ctx->handle.Init.BlockLength 		= 0;
	ctx->handle.Init.AutoRetryCount 	= 0;
	ctx->handle.Init.OneBitSampling 	= SMARTCARD_ONE_BIT_SAMPLE_DISABLE;
	ctx->handle.Init.Prescaler 			= 5;				//should be around 2.4 Mhz (PCLK1 / (PRE * 2))
	ctx->handle.AdvancedInit.AdvFeatureInit = SMARTCARD_ADVFEATURE_NO_INIT;
#endif
#ifdef STM32F4 
	ctx->handle.Init.NACKState = USART_NACK_ENABLE;
	ctx->handle.Init.Prescaler = 5;				//should be around 2.4 Mhz (PCLK1 / (PRE * 2))
#endif
	//if((ctx->protocol & 0x0F) == ICC_PROTOCOL_T1) ctx->handle.Init.GuardTime = ctx->guardtime -1;
	//else ctx->handle.Init.GuardTime = ctx->guardtime;
	ctx->handle.Init.GuardTime = 0;
	if(HAL_SMARTCARD_Init((SMARTCARD_HandleTypeDef *)ctx) != HAL_OK) {
		while(1);
	}
}

//T=0 transmit
uint16 icc_transmit(icc_context * ctx, BYTE * c_apdu, uint16 length, BYTE * r_apdu) {
	uint8 b; uint16 lc = 0;
	uint16 index = 0, le = 2;
	uint32 tickstart = HAL_GetTick();
	if(length == 5) le = 2 + c_apdu[4];
	icc_sendbytes(ctx, c_apdu, 5);
	start_timeout:
	tickstart = HAL_GetTick();
	icc_timeout(ctx, ICC_TIMEOUT_START);
	while(g_bPduTail == g_bPduHead && HAL_GetTick() < (tickstart + 250)) {
		if(icc_active(ctx)) return 0;		//card removed
	}
	icc_timeout(ctx, ICC_TIMEOUT_STOP);
	if(HAL_GetTick() >= (tickstart + 250)) return -1;
	b = icc_receive_byte(ctx);
	if(b == 0x60) goto start_timeout;		//start timeout
	if((b & 0xF0) == 0x60 || (b & 0xF0) == 0x90) { 		//status word
		le = 2; r_apdu[0] = b; index = 1; 
	} else {
		//data/response (datafield processing)
		if((b & 0xFE) == (c_apdu[1] & 0xFE)) {				//data request
			if(length == 5) { 
				if(c_apdu[4] == 0) le = 258;
				goto start_response;
			}
			lc = c_apdu[4];
			if(c_apdu[4] == 0) lc = 256;
			icc_sendbytes(ctx, c_apdu + 5, lc);
			le = 2;
			goto start_timeout;
		}
	}
	start_response:
	icc_timeout(ctx, ICC_TIMEOUT_START);
	for(;index < le;index++) {
		tickstart = HAL_GetTick();
		while(g_bPduTail == g_bPduHead && HAL_GetTick() < (tickstart + 250)) {
			if(icc_active(ctx)) return 0;		//card removed
		}
		//if(g_bIccTimeout) return -1;		//no response (timeout)
		if(HAL_GetTick() >= (tickstart + 250)) return -1;
		b = icc_receive_byte(ctx);
		r_apdu[index] = b;
	}
	icc_timeout(ctx, ICC_TIMEOUT_STOP);
	return index;
}


//scard APIs
/*!
******************************************************************************
\fn 		uint8 if_card_list(scard_context * ctx) 
\brief    	list of all available card reader   
\return   	void
\author   	AGP
\version  	10
\date     	

\verbatim

\endverbatim
******************************************************************************
*/
uint8 if_card_list(scard_context * ctx) { return 0; }

#if SHARD_RTOS_ENABLED 
struct card_async_params {
	uint8 type;
	void * command;
	uint16 cmdlen;
} card_async_params;

void if_card_task() {
	scard_context_p ctx;
	os_message * msg;
	ctx = os_get_context();
	struct card_async_params * params;
	while(1) {
		msg = os_dequeue_message();
		if(msg != NULL) {
			ctx = msg->context;
			msg->reslen = 0;											//reset reslen first
			if(ctx == NULL) goto abort_operation;
			if(msg->reqlen == 0) goto abort_operation;
			params = msg->request;
			if(if_card_state(ctx) != 0) goto abort_operation;		//check for card state
			switch(params->type) {
				case 0:		//reset
					msg->reslen = icc_get_atr(ctx, msg->response, 1000);
					break;
				case 1:		//transmit
					if((ctx->protocol & 0x0F) == ICC_PROTOCOL_T1) 
						msg->reslen = icc_send_iblock(ctx, params->command, params->cmdlen, msg->response);		//use for T=1
					else 
						msg->reslen = icc_transmit(ctx, params->command, params->cmdlen, msg->response);				//default T=0
					break;
				default: break;
			}
			abort_operation:
			os_dispatch_reply(msg);
		}
		os_wait(50);
	}
}
#endif

void if_card_init(void * display, scard_context * ctx) { 
	icc_init(ctx); 

#if SHARD_RTOS_ENABLED 
	//check if card task already created
	if(os_find_task_by_name("card") == NULL) {
		os_create_task(ctx, if_card_task, "card", 29, 1584);		//highest priority task
	}
#endif
}

uint8 if_card_connect(scard_context * ctx, uint8 mode, uint8 * atr) {
#if SHARD_RTOS_ENABLED
	struct card_async_params req;
	os_message * msg;
	uint16 ret;
	uint8 atrlen;
	req.type = 0;		//reset command
	req.command = NULL;
	req.cmdlen = 0;
	msg = os_send_message(os_find_task_by_name("card"), os_create_message(ctx, &req, sizeof(struct card_async_params), atr));
	atrlen = msg->reslen;
	os_delete_message(msg);
	return atrlen;
#else
	return icc_get_atr(ctx, atr, 1000);
#endif
}

uint16 if_card_send(scard_context * ctx, uint8 * c_apdu, uint8 length, uint8 * r_apdu) {
#if SHARD_RTOS_ENABLED
	struct card_async_params req;
	os_message * msg;
	uint16 ret;
	uint16 reslen;
	req.type = 1;		//transmit command
	req.command = c_apdu;
	req.cmdlen = length;
	msg = os_send_message(os_find_task_by_name("card"), os_create_message(ctx, &req, sizeof(struct card_async_params), r_apdu));
	reslen = msg->reslen;
	os_delete_message(msg);
	return reslen;
#else
	if((ctx->protocol & 0x0F) == ICC_PROTOCOL_T1) 
		return icc_send_iblock(ctx, c_apdu, length, r_apdu);		//use for T=1
	else 
		return icc_transmit(ctx, c_apdu, length, r_apdu);				//default T=0
#endif
}

uint8 if_card_state(scard_context * ctx) {
	return icc_active(ctx);
}

uint8 if_card_disconnect(scard_context * ctx, BYTE mode) {
	return 0;
}
