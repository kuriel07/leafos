#include "defs.h"
#include "config.h"
#include "..\inc\MMApis.h"	
#include "..\inc\VMStackApis.h"	
#include "..\crypto\inc\cr_apis.h"
#include "..\toolkit\inc\tk_apis.h"
#include "..\gui\inc\ui_core.h"
#include "..\core\inc\os_core.h"
#include "..\interfaces\inc\if_apis.h"
#include "..\inc\vm_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//extern vm_object * g_pVaRetval;
//extern sys_context g_sVaSysc;
extern tk_context_p g_pfrmctx;


////////////////////////////////////////////IO APIS//////////////////////////////////////////////
//#include "stm32f4xx.h"
//#include "stm32f4xx_hal_conf.h"
/*
<api name="com_open" owb="false" id="192" desc="open specific communication port (UART)" >
    <param name="port" desc="port number" />
    <param name="baud" desc="baud rate(9600, 19200)" />
    <param name="parity" desc="parity mode (0=even, 1=odd)" />
    <param name="stop" desc="stop bit (0 = 1 stop bit, 1 = 1.5 stop bit, 2 = 2 stop bit)" />
  </api>
  <api name="com_close" owb="false" id="193" desc="close an already opened communication port">
    <param name="handle" desc="handle to com port" />
  </api>
  <api name="com_read" owb="false" id="194" desc="read data from communication port">
    <param name="handle" desc="handle to com port" />
    <param name="timeout" desc="timeout in miliseconds" />
  </api>
  <api name="com_send" owb="false" id="195" desc="write data to communication port, can send in sequence of bytes">
    <param name="handle" desc="handle to com port" />
    <param name="data" desc="data to send" />
  </api>
  <api name="io_read" owb="false" id="196" desc="read an io port">
    <param name="port" desc="port number" />
  </api>
  <api name="io_write" owb="false" id="197" desc="write an io port with specific word">
    <param name="port" desc="command to dispatch" />
    <param name="value" desc="qualifier of current command" />
  </api>
*/
//IO APIs

static void va_com_sendbyte(UART_HandleTypeDef * handle, uint8 b) {
   	//handle->Instance->DR = b;
	HAL_USART_WRITE(handle[0], b);
	while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)handle->Instance, USART_FLAG_TXE) == RESET);
   	//while (!(handle->Instance->SR & USART_FLAG_TXE)); 
}

static void va_com_recv(VM_DEF_ARG) {
	//<param name="handle" desc="handle to com port" />
    //<param name="timeout" desc="timeout in miliseconds" />
	OS_DEBUG_ENTRY(va_com_recv);
	UART_HandleTypeDef * handle;
	uint16 i;
	uint8 len;
	uint16 timeout;
	uint8 buffer[256];
	handle = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
	timeout = va_o2f(vm_get_argument(VM_ARG, 1));
	for(i =0;i<timeout; i++) {
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)handle, USART_FLAG_RXNE) == RESET) os_wait(1);
	}
	if(i != timeout) {
		for(i=0;i < 40000 && len < 240;i++) {  
			if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)handle, USART_FLAG_RXNE) != RESET) {
				//buffer[len++] = handle->Instance->DR;
				buffer[len++] = HAL_USART_READ(handle[0]);
				i = 0;
			}
		}
		vm_set_retval(vm_create_object(len, buffer));
	}
	OS_DEBUG_EXIT();
}

static void va_com_send(VM_DEF_ARG) {
	//<param name="handle" desc="handle to com port" />
    //<param name="data" desc="data to send" />
	OS_DEBUG_ENTRY(va_com_send);
	UART_HandleTypeDef * handle;
	vm_object * data;
	uint8 i;
	if(vm_get_argument_count(VM_ARG) >= 2) {
		handle = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
		data = vm_get_argument(VM_ARG, 1);
		for(i=0;i<data->len;i++) {
			va_com_sendbyte(handle, data->bytes[i]);
		}
	}
	OS_DEBUG_EXIT();
}

static void va_com_close(VM_DEF_ARG) {
	//<param name="handle" desc="handle to com port" />
	__HAL_RCC_USART3_CLK_DISABLE();
}

void va_com_init() {
	GPIO_InitTypeDef GPIO_InitStructure;         
	UART_HandleTypeDef usartHandle;
	uint16 baud = 9600, parity = 0, stopbit = 0;
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART3_CLK_ENABLE();
	//icc clock
   	/* USART3 Tx (PD8) */
  	GPIO_InitStructure.Pin = GPIO_PIN_10;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;	 					//default AF_PP
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
	GPIO_InitStructure.Alternate = GPIO_AF7_USART3;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	//GPIO_PinAFConfig(GPIOA, GPIO_PIN_9, GPIO_AF_USART1);
  	/* USART3 Rx (PD9)  */
  	GPIO_InitStructure.Pin = GPIO_PIN_11;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;						//default AF_PP
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;								//default PULLUP
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
	//GPIO_SetBits(GPIOA, GPIO_PIN_10);
	GPIO_InitStructure.Alternate = GPIO_AF7_USART3;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	usartHandle.Instance = USART3;
	usartHandle.Init.BaudRate = baud;
	usartHandle.Init.WordLength 		= USART_WORDLENGTH_8B; 	//8位数据
	switch(stopbit) {
		default:
		case 0: 
			usartHandle.Init.StopBits            = USART_STOPBITS_1;	 		//停止位1位
			break;
		case 1: 
			usartHandle.Init.StopBits            = USART_STOPBITS_1_5;	 		//停止位1位
			break;
		case 2: 
			usartHandle.Init.StopBits            = USART_STOPBITS_2;	 		//停止位1位
			break;
	}
	switch(parity) {
		default:
		case 0: 
			usartHandle.Init.Parity              	= USART_PARITY_NONE ;	 	//无
			break;
		case 1: 
			usartHandle.Init.Parity              	= USART_PARITY_ODD;	 	//无
			break;
		case 2: 
			usartHandle.Init.Parity              	= USART_PARITY_EVEN;	 	//无
			break;
	}	
	usartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	usartHandle.Init.OverSampling = UART_OVERSAMPLING_16;	//UART_OVERSAMPLING_16;
	//USART_InitStructure.HardwareFlowControl = USART_HardwareFlowControl_None;
	usartHandle.Init.Mode = USART_MODE_TX_RX;
	HAL_UART_Init((UART_HandleTypeDef *)&usartHandle);
}

void va_com_open(VM_DEF_ARG) {
	//<param name="port" desc="port number" />
    //<param name="baud" desc="baud rate(9600, 19200)" />
    //<param name="parity" desc="parity mode (0=even, 1=odd)" />
    //<param name="stop" desc="stop bit (0 = 1 stop bit, 1 = 1.5 stop bit, 2 = 2 stop bit)" />   
	OS_DEBUG_ENTRY(va_com_open);
	UART_HandleTypeDef usartHandle;
	uint16 baud, parity, stopbit;
	va_com_context defctx;
	if(vm_get_argument_count(VM_ARG) < 2) goto exit_com_open;
	baud = va_o2f(vm_get_argument(VM_ARG, 1));
	parity = va_o2f(vm_get_argument(VM_ARG, 2));
	stopbit = va_o2f(vm_get_argument(VM_ARG, 3));
	__HAL_RCC_USART3_CLK_ENABLE();
	
	usartHandle.Instance = USART3;
	usartHandle.Init.BaudRate = baud;
	usartHandle.Init.WordLength 		= USART_WORDLENGTH_8B; 	//8位数据
	switch(stopbit) {
		default:
		case 0: 
			usartHandle.Init.StopBits            = USART_STOPBITS_1;	 		//停止位1位
			break;
		case 1: 
			usartHandle.Init.StopBits            = USART_STOPBITS_1_5;	 		//停止位1位
			break;
		case 2: 
			usartHandle.Init.StopBits            = USART_STOPBITS_2;	 		//停止位1位
			break;
	}
	switch(parity) {
		default:
		case 0: 
			usartHandle.Init.Parity              	= USART_PARITY_NONE ;	 	//无
			break;
		case 1: 
			usartHandle.Init.Parity              	= USART_PARITY_ODD;	 	//无
			break;
		case 2: 
			usartHandle.Init.Parity              	= USART_PARITY_EVEN;	 	//无
			break;
	}	
	usartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	usartHandle.Init.OverSampling = UART_OVERSAMPLING_16;	//UART_OVERSAMPLING_16;
	//USART_InitStructure.HardwareFlowControl = USART_HardwareFlowControl_None;
	usartHandle.Init.Mode = USART_MODE_TX_RX;
	HAL_UART_Init((UART_HandleTypeDef *)&usartHandle);
	//((va_default_context *)&defctx)->ctx = ctx;
	((va_default_context *)&defctx)->close = va_com_close;
	((va_default_context *)&defctx)->read = va_com_recv;
	((va_default_context *)&defctx)->write = va_com_send;
	((va_default_context *)&defctx)->offset =  0;
	((va_default_context *)&defctx)->seek = NULL;
	vm_set_retval(vm_create_object(sizeof(va_com_context), &defctx));
	if(vm_get_retval() == VM_NULL_OBJECT) goto exit_com_open;
	((va_com_context *)vm_get_retval()->bytes)->base.ctx = &((va_com_context *)vm_get_retval()->bytes)->handle;
	memcpy(&((va_com_context *)vm_get_retval()->bytes)->handle, &usartHandle, sizeof(UART_HandleTypeDef) );
	exit_com_open:
	OS_DEBUG_EXIT();
	return;
}

void va_com_readline(VM_DEF_ARG) {
	//<param name="ctx" desc="current handle" />
	OS_DEBUG_ENTRY(va_com_readline);
	UART_HandleTypeDef * handle;
	uint8 c;
	uint16 len = 0;
	uint8 buffer[VA_OBJECT_MAX_SIZE];
	if(vm_get_argument_count(VM_ARG) < 1) goto exit_com_readline;
	handle = ((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
	while(1) {
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)handle, USART_FLAG_RXNE) != RESET) {
			///buffer[len++] = c = handle->Instance->DR;
			buffer[len++] = c = HAL_USART_READ(handle[0]);
			if(c == '\n') break;						//newline found
			if(len == sizeof(buffer)) break;		//max buffer reached
		}
	}
	vm_set_retval(vm_create_object(len, buffer));
	exit_com_readline:
	OS_DEBUG_EXIT();
	return;
}
//////////////////////////////////////END OF IO APIS//////////////////////////////////////////
