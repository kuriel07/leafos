#include "defs.h"
#include "config.h"
#include "..\inc\if_apis.h"
#include <string.h>
//#include "stm32f4xx_hal_tim.h"
//initial release (2016.03.18)

timer_context_p g_ltlist = NULL;
TIM_HandleTypeDef g_TIM2_BaseInitStructure;

static void if_timer_start() {
	g_TIM2_BaseInitStructure.Instance = TIM2;
	g_TIM2_BaseInitStructure.Init.Period = 50000;
	g_TIM2_BaseInitStructure.Init.Prescaler = 1000;
	g_TIM2_BaseInitStructure.Init.ClockDivision = 100;
	g_TIM2_BaseInitStructure.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	g_TIM2_BaseInitStructure.Init.RepetitionCounter = 0;
	
	HAL_TIM_Base_Init(&g_TIM2_BaseInitStructure); 
	__HAL_TIM_CLEAR_FLAG(&g_TIM2_BaseInitStructure, TIM_FLAG_UPDATE);
	__HAL_TIM_ENABLE_IT(&g_TIM2_BaseInitStructure, TIM_IT_CC1);
	NVIC_EnableIRQ(TIM2_IRQn);
	//HAL_TIM_Base_Start(&g_TIM2_BaseInitStructure); 
}

static void if_timer_stop() {
	//TIM_Cmd(TIM2, DISABLE); 
	__HAL_TIM_DISABLE(&g_TIM2_BaseInitStructure);
	//TIM_DeInit( TIM2);                              //¸´Î»TIM2¶¨Ê±ÆE
	HAL_TIM_Base_DeInit(&g_TIM2_BaseInitStructure);
	NVIC_DisableIRQ(TIM2_IRQn);
}

void * if_timer_init(void * display) {
	__HAL_RCC_TIM2_CLK_ENABLE();
	if_timer_start();
}

uint8 if_timer_sleep() {
	OS_DEBUG_ENTRY(if_timer_sleep);
	__HAL_RCC_TIM2_CLK_DISABLE();
	OS_DEBUG_EXIT();
	return 0;
}

void if_timer_wake() {
	OS_DEBUG_ENTRY(if_timer_wake);
	__HAL_RCC_TIM2_CLK_ENABLE();
	OS_DEBUG_EXIT();
}
timer_context_p g_cur_timer = NULL;
void * if_timer_create(void (* callback)(void * params), void * params, uint32 tick) {
	timer_context_p tctx;
	timer_context_p iterator;
	if(callback == NULL) return NULL;
	tctx = (timer_context_p)malloc(sizeof(timer_context));
	tctx->magic = 0xAA;
	tctx->callback = callback;
	tctx->curtimer = 0;
	tctx->cmptimer = tick;
	tctx->next = NULL;
	tctx->params = params;
	if(g_ltlist == NULL) g_ltlist = tctx;
	else {
		iterator = g_ltlist;
		while(iterator->next != NULL) iterator = iterator->next;
		iterator->next = tctx;
	}
	return tctx;
}

void if_timer_delete(void * tctx) {
	timer_context_p iterator;
	if(g_ltlist == NULL) return;
	if(g_ltlist == tctx) { 
		g_ltlist = ((timer_context_p)tctx)->next;
		free(tctx);
	} else {
		iterator = g_ltlist;
		while(iterator->next != NULL) {
			if(iterator->next == tctx) { 
				iterator->next = ((timer_context_p)tctx)->next;
				free(tctx);
			}
			iterator = iterator->next;
		}
	}
}

static void if_timer_tick() {
	timer_context_p iterator = g_ltlist;
	while(iterator != NULL) {
		if(iterator->magic != 0xAA) break;		//invalid timer
		if(iterator->curtimer == iterator->cmptimer) {
			iterator->curtimer = 0;
			//execute timer handler
			g_cur_timer = iterator;
			if(iterator->callback != NULL) iterator->callback(iterator->params);
		} else {
			iterator->curtimer++;
		}
		iterator = iterator->next;
	}
	g_cur_timer = NULL;
}

void if_timer_set_tick(uint16 tick) {
	if(g_cur_timer == NULL) return;
	g_cur_timer->cmptimer = tick;
}

void TIM2_IRQHandler(void) 
{
	if_timer_tick();
	//TIM_ClearFlag(TIM2, TIM_FLAG_CC1); 
	__HAL_TIM_CLEAR_FLAG(&g_TIM2_BaseInitStructure, TIM_FLAG_CC1);
}