#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\if_apis.h"
#include <string.h>
#include "..\inc\if_pwm.h"
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif

pwm_context_p g_pwmlist = NULL;
TIM_HandleTypeDef g_TIM8_BaseInitStructure;

void if_pwm_init(void * context) {
	uint32 freq = HAL_RCC_GetPCLK1Freq();
	uint32 period = freq / PWM_MAX_PERIOD;
	g_TIM8_BaseInitStructure.Instance = TIM8;
	g_TIM8_BaseInitStructure.Init.Period = period/100;
	g_TIM8_BaseInitStructure.Init.Prescaler = 1;
	g_TIM8_BaseInitStructure.Init.ClockDivision = 100;
	g_TIM8_BaseInitStructure.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	g_TIM8_BaseInitStructure.Init.RepetitionCounter = 0;
	
	__HAL_RCC_TIM8_CLK_ENABLE();
	HAL_TIM_Base_Init(&g_TIM8_BaseInitStructure); 
	__HAL_TIM_CLEAR_FLAG(&g_TIM8_BaseInitStructure, TIM_FLAG_UPDATE);
	__HAL_TIM_ENABLE_IT(&g_TIM8_BaseInitStructure, TIM_IT_CC1);
	NVIC_EnableIRQ(TIM8_CC_IRQn);
	//HAL_TIM_Base_Start(&g_TIM8_BaseInitStructure); 
}

static void if_pwm_tick() {
	static uint32 pwm_tick = 0;
	pwm_context_p iterator = g_pwmlist;
	while(iterator != NULL) {
		if(iterator->magic != 0xAC) break;		//invalid timer
		if((iterator->state & (PWM_STATE_INITIALIZED | PWM_STATE_STARTED)) == (PWM_STATE_INITIALIZED | PWM_STATE_STARTED)) {
			if(pwm_tick == 0) {
				HAL_GPIO_WritePin(iterator->handle, iterator->pin, GPIO_PIN_SET);
			}
			if(pwm_tick == iterator->p1) {
				HAL_GPIO_WritePin(iterator->handle, iterator->pin, GPIO_PIN_RESET);
			}
		}
		iterator = iterator->next;
	}
	pwm_tick ++;
	if(pwm_tick > PWM_MAX_PERIOD) pwm_tick = 0;
}

void TIM8_CC_IRQHandler(void) 
{
	if_pwm_tick();
	__HAL_TIM_CLEAR_FLAG(&g_TIM8_BaseInitStructure, TIM_FLAG_CC1);
}

//resolution 10 tick/Hz;
pwm_context_p if_pwm_create(GPIO_TypeDef * handle, uint16 pin, uint16 dutycycle, uint16 freq) {
	pwm_context_p ctx;
	pwm_context_p iterator;
	GPIO_InitTypeDef GPIO_InitStructure;
	uint32 temp;
	if(freq > PWM_MAX_FREQ) return NULL;
	if(dutycycle > 100) return NULL;
	ctx = (pwm_context_p)os_alloc(sizeof(pwm_context));
	ctx->state = 0;
	ctx->magic = 0xAC;
	ctx->handle = handle;
	ctx->pin = pin;
	ctx->state = PWM_STATE_INITIALIZED;
	temp = (dutycycle * PWM_MAX_PERIOD) / 100;
	ctx->p1 = temp;
	ctx->next = NULL;
	if(g_pwmlist == NULL) g_pwmlist = ctx;
	else {
		iterator = g_pwmlist;
		while(iterator->next != NULL) iterator = iterator->next;
		iterator->next = ctx;
	}
	//initialize output pin
	GPIO_InitStructure.Pin = pin;	//CK
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;	
  	HAL_GPIO_Init(handle, &GPIO_InitStructure);	
	return ctx;
}

void if_pwm_set_dutycycle(pwm_context_p ctx, uint8 dutycycle) {
	uint32 temp;
	if(dutycycle > 100) return;
	temp = (dutycycle * PWM_MAX_PERIOD) / 100;
	ctx->p1 = temp;
}

void if_pwm_delete(void * tctx) {
	pwm_context_p iterator;
	if(g_pwmlist == NULL) return;
	if(g_pwmlist == tctx) { 
		g_pwmlist = ((pwm_context_p)tctx)->next;
		os_free(tctx);
	} else {
		iterator = g_pwmlist;
		while(iterator->next != NULL) {
			if(iterator->next == tctx) { 
				iterator->next = ((pwm_context_p)tctx)->next;
				os_free(tctx);
				return;
			}
			iterator = iterator->next;
		}
	}
}

void if_pwm_start(pwm_context_p ctx) {
	if(ctx == NULL) return;
	if(ctx->magic != 0xAC) return;
	ctx->state |= PWM_STATE_STARTED;
}

void if_pwm_stop(pwm_context_p ctx) {
	if(ctx == NULL) return;
	if(ctx->magic != 0xAC) return;
	ctx->state &= ~PWM_STATE_STARTED;
}

void if_pwm_release(pwm_context_p ctx) {
	if_pwm_delete(ctx);
}