#include "defs.h"
#include "config.h"
#include "..\inc\if_host.h"
#include <string.h>

static const unsigned char SIO_BYTE_MASK[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 }; 
unsigned short g_ht_state = HS_INIT;
unsigned char g_htbuffer[256];
unsigned char g_httail = 0;
unsigned char g_hthead = 0;
unsigned short g_hr_state = HS_INIT;
unsigned char g_hrbuffer[256];
unsigned char g_hrtail = 0;
unsigned char g_hrhead = 0;
unsigned int g_hrtimeout = 2064;
static host_context_p g_cur_context = NULL;
static void MX_TIM3_Init(void);
TIM_HandleTypeDef htim3;

void host_init(host_context_p ctx) {
	GPIO_InitTypeDef GPIO_InitStruct;
	if(ctx == NULL) return;
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	ctx->base = GPIOE;
	ctx->sio = GPIO_PIN_10;
	ctx->clk = GPIO_PIN_11;
	g_cur_context = ctx;
	
	
	GPIO_InitStruct.Pin = ctx->sio;			//host dio
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(ctx->base, &GPIO_InitStruct);
	/*Configure GPIO pin : PA7 */
	GPIO_InitStruct.Pin = ctx->clk;			//host clk
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(ctx->base, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(ctx->base, ctx->sio, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ctx->base, ctx->clk, GPIO_PIN_RESET);		//clk should stay low
	HAL_Delay(100);
	HAL_GPIO_WritePin(ctx->base, ctx->sio, GPIO_PIN_SET);
	MX_TIM3_Init();
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	
	g_ht_state = HS_WAIT_SOF;
	g_hr_state = HS_WAIT_SOF;
}

void host_deinit(host_context_p ctx) {
	
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_TIM_Base_Stop_IT(&htim3);
	__HAL_RCC_TIM3_CLK_DISABLE();
}

void host_send(host_context_p ctx, unsigned char * buffer, unsigned length) {
	memcpy(g_htbuffer, buffer, length);
	g_hthead = 0;
	g_httail = length;
	
	while(g_httail != g_hthead);
	g_hthead = 0;
	g_httail = 0;
}

unsigned char host_recv(host_context_p ctx, unsigned char * buffer) {
	unsigned char len;
	g_hrtimeout = 2064;
	while(g_hrtail == 0 && g_hrtimeout != 0);
	while(g_hrhead != g_hrtail && g_hrtimeout != 0);
	len = g_hrtail;
	memcpy(buffer, g_hrbuffer, len);
	g_hrtail = 0;
	g_hrhead = 0;
	if(g_hrtimeout == 0) return 0;
	//wait for SOF state before exiting
	while((g_hr_state & 0xFF00) != HS_WAIT_SOF);	
	return len;
}

static void host_setup() {
	unsigned char len;
	static unsigned char wait_space;
	//setup next data
	if(g_cur_context == NULL) return;
	if(g_ht_state == HS_INIT) return;
	if(g_hthead == g_httail) return;
	len = (g_httail - g_hthead);
	switch(g_ht_state & 0xFF00) {
		case HS_WAIT_SOF:
			HAL_GPIO_WritePin(g_cur_context->base, g_cur_context->sio, GPIO_PIN_RESET);
			g_ht_state = HS_TRANSMIT | HS_LEN | HS_BIT_0;
			break;
		case HS_LEN | HS_TRANSMIT:			//transmit length
			if(SIO_BYTE_MASK[g_ht_state & 0x07] & len) HAL_GPIO_WritePin(g_cur_context->base, g_cur_context->sio, GPIO_PIN_SET);
			else HAL_GPIO_WritePin(g_cur_context->base, g_cur_context->sio, GPIO_PIN_RESET);
			g_ht_state++;
			if((g_ht_state & 0x07) == 0) g_ht_state = HS_TRANSMIT | HS_BIT_0;		//switch state
			break;
		case HS_TRANSMIT:						//transmit data
			if(SIO_BYTE_MASK[g_ht_state & 0x07] & g_htbuffer[g_hthead]) HAL_GPIO_WritePin(g_cur_context->base, g_cur_context->sio, GPIO_PIN_SET);
			else HAL_GPIO_WritePin(g_cur_context->base, g_cur_context->sio, GPIO_PIN_RESET);
			g_ht_state++;
			if((g_ht_state & 0x07) == 0) {
				g_hthead++;
				wait_space = 0;
				if(g_hthead == g_httail) g_ht_state = HS_SPACE;	//wait for specified amount of bits
				else g_ht_state = HS_TRANSMIT | HS_BIT_0;		//transmit next byte
			}
			break;
		case HS_SPACE:
			HAL_GPIO_WritePin(g_cur_context->base, g_cur_context->sio, GPIO_PIN_SET);
			wait_space++;
			if(wait_space == 64) g_ht_state = HS_WAIT_SOF;
			break;
	}
}

static void host_process() {
	GPIO_PinState bit;
	GPIO_InitTypeDef GPIO_InitStruct;
	static unsigned char start_sampling_8 = 0;
	static unsigned char len = 0;
	static unsigned char dat = 0;
	static unsigned short wait_cntr = 0;
	static unsigned char wait_space;
	unsigned short to = 100;
	if(g_cur_context == NULL) return;
	if(g_hr_state == HS_INIT) return;
	//sampling data, advanced bit pointer
	GPIO_InitStruct.Pin = g_cur_context->sio;						//set as input
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(g_cur_context->base, &GPIO_InitStruct);
	
	g_hrtimeout--;		//decrement timeout
	//start sampling
	while(to--);
	bit = HAL_GPIO_ReadPin(g_cur_context->base, g_cur_context->sio);
	
	GPIO_InitStruct.Pin = g_cur_context->sio;						//set as output
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(g_cur_context->base, &GPIO_InitStruct);
	//set high
	HAL_GPIO_WritePin(g_cur_context->base, g_cur_context->sio, GPIO_PIN_SET);
	//re-synchronization (in-case communication lost)
	if((g_hr_state & 0xFF00) != HS_WAIT_SOF) {
		if(bit == GPIO_PIN_SET) {
			wait_cntr++;
			if(wait_cntr == 2064) { 
				g_hr_state = HS_WAIT_SOF;
				wait_cntr = 0;
				g_hrtimeout = 2064;
			}
		} else {
			wait_cntr = 0;
			g_hrtimeout = 2064;
		}
	}
	//start processing bit
	switch(g_hr_state & 0xFF00) {
		case HS_WAIT_SOF:
			len = 0;
			if(bit == GPIO_PIN_RESET) g_hr_state = HS_LEN | HS_RECEIVE | HS_BIT_0;
			break;
		case HS_LEN | HS_RECEIVE:
			if(bit == GPIO_PIN_SET) len |= SIO_BYTE_MASK[g_hr_state & 0x07];
			g_hr_state++;
			if((g_hr_state & 0x07) == 0) {
				g_hrtail = len;
				g_hrhead = 0;
				dat = 0;
				g_hr_state = HS_RECEIVE | HS_BIT_0;
			}
			break;
		case HS_RECEIVE:
			if(bit == GPIO_PIN_SET) dat |= SIO_BYTE_MASK[g_hr_state & 0x07];
			g_hr_state++;
			if((g_hr_state & 0x07) == 0) {
				g_hrbuffer[g_hrhead] = dat;
				g_hrhead++;
				dat = 0;
				if(g_hrhead == g_hrtail) g_hr_state = HS_SPACE;		//no need to add space
				else g_hr_state = HS_RECEIVE | HS_BIT_0;
			}
			break;
		case HS_SPACE:			//add 64 bit clock but ignore the bit
			wait_space++;
			if(wait_space == 64) g_hr_state = HS_WAIT_SOF;
			break;
	}
}

//master handler
void TIM3_IRQHandler(void) {
	static unsigned int i=0;
	static GPIO_PinState clk = GPIO_PIN_RESET;
	unsigned short to = 100;
	if(i < 10) {
		i++;
		return;
	}
	HAL_TIM_IRQHandler(&htim3);
	if(clk == GPIO_PIN_SET) {
		host_setup();			//setup next data
		while(to--);
	}
	HAL_GPIO_WritePin(g_cur_context->base, g_cur_context->clk, clk);
	if(clk == GPIO_PIN_RESET) {
		host_process();			//sampling data, advanced bit pointer
	}
	clk = !clk;
}		


/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_SlaveConfigTypeDef sSlaveConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 5;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim3.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
		//Error_Handler();
	  while(1);
  }
}
