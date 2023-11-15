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

va_port_config g_portconf[] = {
		{ 0, GPIOE, 3 },
		{ 1, GPIOE, 4 },
		{ 2, GPIOE, 5 },
		{ NULL, NULL, NULL }		//end mark
};

uint16 g_pinconf[] = {
	GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3,
	GPIO_PIN_4,GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, 
	GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11,
	GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
};

static void va_port_read(VM_DEF_ARG) {
	//<param name="port" desc="port number" />
	OS_DEBUG_ENTRY(va_port_read);
	uint8 buffer[8];
	GPIO_InitTypeDef GPIO_InitStructure;     
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	uint16 pin;
	GPIO_TypeDef * port;
	uint16 value;
	
	port = (GPIO_TypeDef *)((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
	pin = ((va_port_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->pin;
	switch( ((va_port_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->state) {
		case VA_PINTYPE_IO:
			GPIO_InitStructure.Pin = g_pinconf[pin];
			GPIO_InitStructure.Mode = GPIO_MODE_INPUT;	 					//default AF_PP
			GPIO_InitStructure.Pull = GPIO_NOPULL;								//default NOPULL
			GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
			HAL_GPIO_Init(port, &GPIO_InitStructure);
			value = HAL_GPIO_ReadPin(port, g_pinconf[pin]);
			if(value != 0) value = 1;														//set value to 1
			GPIO_InitStructure.Pin = g_pinconf[pin];
			GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;	 					//default AF_PP
			//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
			GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
			HAL_GPIO_Init(port, &GPIO_InitStructure);			//set as output
			sprintf((char *)buffer, "%d", value);
			vm_set_retval(vm_create_object(strlen((const char *)buffer), buffer));
			break;
		default:		//PWM, etc...
			break;
	}
	OS_DEBUG_EXIT();
}

static void va_port_write(VM_DEF_ARG) {
	//<param name="ctx" desc="command to dispatch" />
    //<param name="value" desc="qualifier of current command" />
	OS_DEBUG_ENTRY(va_port_write);
	GPIO_InitTypeDef GPIO_InitStructure;     
	vm_object * vctx = vm_get_argument(VM_ARG, 0);
	GPIO_TypeDef * port;
	uint16 pin;
	uint16 value = va_o2f(vm_get_argument(VM_ARG, 1));
	//PD9=0 PD8=1 PE5=2
	port = (GPIO_TypeDef *)((va_default_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->ctx;
	pin = ((va_port_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->pin;
	
	//HAL_GPIO_WritePin(GPIOE, value, GPIO_PinState PinState);
	switch( ((va_port_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->state) {
		case VA_PINTYPE_IO:
			if(value == 0) {
				port->ODR &= ~g_pinconf[pin];
			} else {
				port->ODR |= g_pinconf[pin];
			}
			break;
		case VA_PINTYPE_PWM:
			if(value == 0) {
				if_pwm_stop(((va_port_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->handle);
			} else {
				if_pwm_start(((va_port_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes)->handle);
			}
			break;
	}
	OS_DEBUG_EXIT();
}

static void va_port_seek(VM_DEF_ARG) {
	
}

static void va_port_close(VM_DEF_ARG) {
	//release any handle if exist
	uint8 i = 0;
	GPIO_InitTypeDef GPIO_InitStructure;  
	va_port_context * pctx = (va_port_context *)((vm_object *)vm_get_argument(VM_ARG, 0))->bytes;
	//disable PWM
	if(pctx->state == VA_PINTYPE_PWM) {
		if(pctx->handle != NULL) {
			if_pwm_release(pctx->handle);
			pctx->handle = NULL;
		}
	}
	//disable output/interrupt
	while(g_portconf[i].port != NULL) {					//check for context if not null
		if((pctx->id & 0x7F) == g_portconf[i].id) {						//check for pin identifier
			GPIO_InitStructure.Pin = g_pinconf[g_portconf[i].pin];
			GPIO_InitStructure.Mode = GPIO_MODE_INPUT;	 					//default AF_PP
			GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
			GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
			HAL_GPIO_Init(g_portconf[i].port, &GPIO_InitStructure);			//set as output
		}
		i++;
	}
}

void va_port_open(VM_DEF_ARG) {
	//param1 = port number
	OS_DEBUG_ENTRY(va_port_open);
	GPIO_InitTypeDef GPIO_InitStructure;    
	va_port_context defctx; 
	uint8 i = 0;
	uint8 port_type = 0;
	void * handle = NULL;
	uint8 port = va_o2f(vm_get_argument(VM_ARG, 0));
	port_type = port & 0x80;			//0x00 = output pin 0, 0x80 input pin 0
	port = port & 0x7F;
	//iterate registered pin
	while(g_portconf[i].port != NULL) {					//check for context if not null
		if(port == g_portconf[i].id) {						//check for pin identifier
			((va_default_context *)&defctx)->ctx = g_portconf[i].port;		//current port
			((va_default_context *)&defctx)->close = va_port_close;
			((va_default_context *)&defctx)->read = va_port_read;
			((va_default_context *)&defctx)->write = va_port_write;
			((va_default_context *)&defctx)->offset =  0;
			((va_default_context *)&defctx)->seek = va_port_seek;
			defctx.id = port;
			defctx.pin = g_portconf[i].pin;			//current pin
			if(vm_get_argument_count(VM_ARG) < 2) {
				set_plain_signal:
				defctx.state = VA_PINTYPE_IO;			//IO port
			} else {
				defctx.freq = 1;							//default 1 hertz
				if(vm_get_argument_count(VM_ARG) >= 2) {
					defctx.freq = va_o2f(vm_get_argument(VM_ARG, 1));
					if(defctx.freq == 0) goto set_plain_signal;				//invalid frequency
					if(defctx.freq > 1000) defctx.freq = 1000;
				}
				defctx.dutycycle = 50;					//default dutycycle
				if(vm_get_argument_count(VM_ARG) >= 3) {
					defctx.dutycycle = va_o2f(vm_get_argument(VM_ARG, 2));
					if(defctx.dutycycle == 0) goto set_plain_signal;		//invalid duty cycle
					if(defctx.dutycycle > 100) defctx.dutycycle = 100;
				}
				defctx.state = VA_PINTYPE_PWM;
			}
			defctx.handle = NULL;
			vm_set_retval(vm_create_object(sizeof(va_port_context), &defctx));
			break;
		}
		i++;
	}
	if(port_type == 0x00) {
		//output type
		GPIO_InitStructure.Pin = g_pinconf[g_portconf[i].pin];
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;	 					//default AF_PP
		GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
		HAL_GPIO_Init(g_portconf[i].port, &GPIO_InitStructure);			//set as output
		//init port PD9, PD8, PE5 as output
		if(defctx.state == VA_PINTYPE_PWM) {
			//set current handle for PWM activity
			handle = if_pwm_create(g_portconf[i].port, g_pinconf[g_portconf[i].pin], defctx.dutycycle, defctx.freq);
			if(handle != NULL && vm_get_retval() != NULL) {
				((va_port_context *)vm_get_retval()->bytes)->handle = handle;		//set current handle
			}
		} 
	} else {
		//input type
		GPIO_InitStructure.Pin = g_pinconf[g_portconf[i].pin];
		GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;	 					//default falling interrupt
		GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
		HAL_GPIO_Init(g_portconf[i].port, &GPIO_InitStructure);			//set as input
		HAL_NVIC_EnableIRQ( EXTI9_5_IRQn);							//enable interrupt
	}
	OS_DEBUG_EXIT();
}

void EXTI9_5_IRQHandler() {
	uint16 code_offset;
	vf_handle orcfile;
	vm_instance * instance = NULL;
	uint8 i = 0;
	uint8 tbuf[5];
	char name[16];
	uint8 event = VM_EVENT_IO;
	if(g_pfrmctx == NULL) return;
	while(g_portconf[i].port != NULL) {					//check for context if not null
		if(__HAL_GPIO_EXTI_GET_IT(g_pinconf[g_portconf[i].pin]) != RESET) {
			__HAL_GPIO_EXTI_CLEAR_IT(g_pinconf[g_portconf[i].pin]);		//clear interrupt request
			//check whether the vm is already running
			if(g_pfrmctx->cos_status & TK_COS_STAT_VM_STARTED) break;
			//start invoke event
			if(tk_kernel_framework_load(g_pfrmctx, &orcfile) != (uint8)-1) {
				code_offset = vr_load_script(VM_SCRIPT_BY_EVENT, &orcfile, 1, &event);
				if(code_offset == (uint16)-1) goto exit_invoke_event;
				//initialize virtual machine
				sprintf(name, "port%d", g_portconf[i].pin);
				instance = vm_new_instance((uint8 *)name);
				if(instance != NULL) {
					vm_init(instance, &orcfile, code_offset);
					//push port_id as argument
					sprintf((char *)tbuf, "%d", g_portconf[i].id);
					vm_push_argument(instance, strlen((const char *)tbuf), tbuf);
					start_execute_app:
					vm_exec_instance(instance);
				}
			}
			exit_invoke_event:
			break;
		}
		i++;
	}
}