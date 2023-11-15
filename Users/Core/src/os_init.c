#include "defs.h"
#include "config.h"
#include "..\..\crypto\inc\cr_apis.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\..\gp\inc\gp_apis.h"
#include "..\..\build.h"
#include "..\inc\os_mach.h"
#include "..\inc\os.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tk_context_p g_current_context = NULL;

void os_init_machine(void * ctx) {
	g_current_context = (tk_context_p)ctx;
	if_sys_init(NULL);
	//NVIC_EnableIRQ(TIM1_CC_IRQn);
}

extern OSTask * _active_task;
extern OSTask * _highest_task, * _lowest_task;
void os_start(void) {				//start task scheduler 
	_active_task = _highest_task;
	SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
}

//disable timer interrupt
uint32 os_enter_critical(void) {
	uint32 ret = SysTick->CTRL;
	if((ret & SysTick_CTRL_ENABLE_Msk) != 0) { 
		while(SysTick->VAL < 200);		//value too small, wait until systick_isr is triggered (2018.10.15)
	}
	SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk);
	__asm
	{
	 	nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	};
	return ret;
}

//enable timer interrupt
void os_exit_critical(uint32 val) {
	SysTick->CTRL = val;
}

void os_clear_int_flag(void) {
 	//ClearPending(BIT_TIMER4);
	//TIM_ClearFlag(TIM1, TIM_FLAG_CC1); 
}



void os_data_abort_exception_callback(uint32 paddr, uint32 daddr) {
	uint8 buffer[256];
	//snprintf(buffer, 256, "[0x%x] data access violation : 0x%x", paddr, daddr);
	//kernel_show_notification(buffer);
	//wm_kill_application(os_get_active_task());
}

void os_program_exception_callback(uint32 paddr) {
	uint8 buffer[256];
	//snprintf(buffer, 256, "[0x%x] program access violation!", paddr);
	//kernel_show_notification(buffer);
	//wm_kill_application(os_get_active_task());
}

void os_switch_cpu_clock(uint8 mode) {
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	__HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_HSI);
	if(mode < 0x7F) {
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
		RCC_OscInitStruct.HSEState = RCC_HSE_ON;
		RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
		RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
		RCC_OscInitStruct.PLL.PLLM = 4;
		RCC_OscInitStruct.PLL.PLLN = 192;
		RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
		RCC_OscInitStruct.PLL.PLLQ = 8;
		RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
		RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
		RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV8;
	} else {
		//automatically switch to voltage scale 3
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
		RCC_OscInitStruct.HSEState = RCC_HSE_ON;
		RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
		RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
		RCC_OscInitStruct.PLL.PLLM = 8;
		RCC_OscInitStruct.PLL.PLLN = 96;
		RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV8;
		RCC_OscInitStruct.PLL.PLLQ = 4;
		RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
		RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
		RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
		RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	}
	//for(i=0;i<0x8000;i++);
	//for(i=0;i<0x8000;i++);
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
	{	
		//Error_Handler();
		while(1);
	}
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		//Error_Handler();
		while(1);
	}
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
}

//implemented 2017.04.08
void os_undefined_exception_callback(uint32 paddr) {
	//uint8 buffer[400];
	uint8 cbuf[1024];
	uint8 * ptr;
	uint16 port = 80;
	uint8 host[32];
	uint8 esid_buf[40];
	uint16 length, hdrlen;
	uint8_c url[] = "http://orbleaf.com/updates/kron/v10/report.php";
	tk_context_p ctx;
	uint32 i, j;
	os_task * task;
	os_task * active = os_get_active_task();
	gui_handle_p display;
	uint32 cpu_sr;
	net_conn_p conn;
#if SHARD_RTOS_ENABLED
	cpu_sr = os_enter_critical();
#endif
	sprintf((char *)gba_net_buffer, "[SYSTEM EXCEPTION]\nKron %d.%d.%d\nactive task : %s\nstack trace : %08x\nbase stack : %08x\nstack size : %d\n\n\nplease wait\nwhile system restarting....", 
							MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER,
							active->name, 
							paddr, 
							((os_thread *)active)->stack,
							((os_thread *)active)->stack_size);
	if(g_current_context != NULL) {
		display = (gui_handle_p)((tk_context_p)g_current_context)->display;
		if(display != NULL) {
			display->fill_area(display, UI_COLOR_RED, display->set_area(display, 0, 0, display->width, display->height));
			display->print_string(display, UI_FONT_DEFAULT, 4, 4, gba_net_buffer, UI_COLOR_WHITE);
		}
		//cannot use normal http request which based on async task, use direct method (2017.04.09)
		ptr = net_decode_url((uint8 *)url, &port, host);
		//nextptr = (uint8 *)strstr((char *)ptr, (const char *)"?");
		//if(nextptr != NULL) nextptr[0] = 0;		//delete host argument (GET??)
		//generate POST payload (parameters)
		tk_bin2hex(g_current_context->esid, SHARD_ESID_SIZE, esid_buf);
		sprintf((char *)gba_net_buffer, "os_ver=%d.%d.%d&task=%s&stack_trace=%08x&stack_base=%08x&stack_size=%d\r\n\r\nesid=%s\r\n\r\ncall_stack : \r\n", 
							MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER,
							active->name, 
							paddr, 
							((os_thread *)active)->stack,
							((os_thread *)active)->stack_size,
							esid_buf);
		//dump call stack tree
		for(i=0;i<active->dbg_index;i++) {
			strcat((char *)gba_net_buffer, (char *)active->dbg_stack[i]);
			strcat((char *)gba_net_buffer, "\r\n");
		}
		
		snprintf((char *)cbuf, sizeof(cbuf), "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n", ptr, host, strlen((const char *)gba_net_buffer)); 
		//calculate header length
		hdrlen = strlen((const char *)cbuf);
		memcpy(cbuf + hdrlen, gba_net_buffer, strlen((const char *)gba_net_buffer));
		strcat((char *)cbuf, "\r\n");
		NET_LOCK_CRITICAL(g_current_context->netctx);
		length = strlen((const char *)cbuf);
		//check if driver available
		if(g_current_context->netctx->init != NULL) {
			//start sending error report
			if((conn = g_current_context->netctx->sock_open(g_current_context->netctx, host, port, NET_TRANSMIT)) != NULL)  {		//try open connection
				g_current_context->netctx->sock_send(conn, cbuf, length);			//try sending POST request
				g_current_context->netctx->sock_close(conn);								//no need to wait for response, close connection
			}
		}
		NET_UNLOCK_CRITICAL(g_current_context->netctx);
	}
	//}
	//manual delay (added 2017.04.08)
#if 0
	for(i=0;i<4370000;i++) {		// for(j=0;j<100;j++);
		__nop();			//4000000 bayar cc
		__nop();			// 370000 rollup banner
		__nop();			
		__nop();			
		for(j=0;j<120;j++);
	}
#endif
#if SHARD_RTOS_ENABLED
	os_exit_critical(cpu_sr);
#endif
	NVIC_SystemReset();
}

