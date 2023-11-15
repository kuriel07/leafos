#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\if_apis.h"
#include "..\inc\if_usb.h"
#include <string.h>
#include "usb_device.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_customhid.h"
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"

//SYNOPSIS STM32 IMPLEMENTATION
//ENDPOINT 0 -> CONTROL IN/OUT						(CURRENT DEFAULT SETUP)
//ENDPOINT 1 -> INTERRUPT IN/BULK (IN/OUT)		(CURRENT INTERRUPT IN)		<- interrupt only available on EP1
//ENDPOINT 2 -> BULK (IN/OUT)							(CURRENT BULK IN)
//ENDPOINT 3 -> BULK (IN/OUT)							(CURRENT BULK OUT)

typedef struct usb_msg_param {
	uint8 endpoint;
	uint8 message[65];
	uint16 msg_len;
} usb_msg_param;
TIM_HandleTypeDef htim11;
static void MX_TIM11_Init(void);

//uint8 gba_usb_buffer[360] __packed;
//uint16 g_usb_buffer_index = 0;

usb_context_p g_curUsbCtx = NULL;
static uint16 if_usb_read(struct usb_context * uctx, uint8 * buffer) {
	uint16 size = 0;
	//if(USBD_LL_PrepareReceive(uctx->instance, 0x03, buffer, 255) == USBD_OK) {
	//	size = USBD_LL_GetRxDataSize(uctx->instance, 0x03);
	//}
	///////////////SYNOPSIS STM32 CUSTOM HID///////////////////////
	//buffer already received by CUSTOM_HID_OutEvent_FS on usbd_custom_hid_if.c
	return USBD_LL_GetRxDataSize(uctx->instance, 0x03);
	///////////////END OF SYNOPSIS STM32 CUSTOM HID////////////////
	//return size;
}

static uint16 if_usb_write(struct usb_context * uctx, uint8 * buffer, uint16 size) {
#if SHARD_RTOS_ENABLED
	static usb_msg_param params[6];
	uint16 i;
	uint8 j;
	uint16 length;
	os_task *task = os_find_task_by_name("usb");
	for(i=0,j=0;i<size;i+=64,j++) {
		length = (size < (i + 64))?size-i:64;
		params[j].endpoint = 0x82;
		memcpy(params[j].message, buffer + i, length);
		params[j].msg_len = length;
		os_queue_message(task, os_create_message(uctx, &params[j], sizeof(usb_msg_param), buffer));
	}
	os_resume(task);
#else
	if(USBD_LL_Transmit(uctx->instance, 0x82, buffer, length) == USBD_OK) return length;
	return 0;
#endif
}

static void if_usb_status(struct usb_context * uctx, uint8 status) {
	uint8 buffer[2] = { 0x50, 0x02 };
	uint32 psw;
	if(status == 0) { buffer[1] = 0x03; }
	USBD_CUSTOM_HID_SendReport(uctx->instance, buffer, 2);
}

static uint16 if_usb_debug(struct usb_context * uctx, uint8 * buffer, uint16 size) {
	//if(size > 0x80) size = 0x80;
	//USBD_CUSTOM_HID_SendReport(uctx->instance, buffer, size);
	static usb_msg_param params[6];
	uint16 i;
	uint8 j;
	uint16 length;
	os_task *task = os_find_task_by_name("usb");
	for(i=0,j=0;i<size;i+=64,j++) {
		length = (size < (i + 64))?size-i:64;
		params[j].endpoint = 0x81;
		//params[j].message = buffer + i;
		memcpy(params[j].message, buffer + i, length);
		params[j].msg_len = length;
		//os_queue_message(task, );
		os_send_message(task, os_create_message(uctx, &params[j], sizeof(usb_msg_param), buffer));
	}
	//os_resume(task);
	return 0;
}

static uint8 if_usb_checksum(uint8 * buffer, uint16 len) {
	uint8 chksum = 0;
	uint16 i;
	for(i=0;i<len;i++) chksum ^= buffer[i];
	return chksum;
}

#if RTK_ENABLE_MONITOR
void os_mon_callback(uint8 * buffer, uint16 size) {
	uint16 len;
	if(g_curUsbCtx == NULL) return;
	len = g_curUsbCtx->debug(g_curUsbCtx, buffer, size);
}
#endif

#if (SHARD_USB_DRIVER == SHARD_DRIVER_OKEY)
void if_usb_handler(usb_context_p uctx, uint32 event) {
	//called when usb receiving command
	uint8 cmdbuf[64];
	uint8 resbuf[64];
	uint16 len;
	uint16 i;
	uint8 c;
	memset(resbuf, 0, sizeof(resbuf));
	if(event == USB_EVT_OUT) {
		len = uctx->read(uctx, cmdbuf);
		memcpy(resbuf + 2, cmdbuf + 2, 5);		//copy sequence number
		switch(cmdbuf[0]) {
			case 0x6B:			//send apdu
				resbuf[0] = 0x83;				//response tag
				if(cmdbuf[1] < 6) {
					resbuf[7] = 0x01;
					resbuf[1] = cmdbuf[1] - 1;
					memcpy(resbuf + 10, cmdbuf + 10, resbuf[1]);		//copy command to response buf
				} else {
					//APDU handler here (partial)
					if(uctx->cardCtx == NULL) break;
					resbuf[10] = cmdbuf[10];	
					//only when T=1
					if(cmdbuf[1] == 6 && memcmp(cmdbuf + 12, "\xc1\x01\xFE\x3E", 4) == 0) {
						memcpy(resbuf + 12, cmdbuf + 12, 4);
						resbuf[12] = 0xE1;
						resbuf[15] = 0x1E;
						resbuf[1] = cmdbuf[1];
						break;
					}
					i = 10;
					c = resbuf[i++];
					switch(c) {
						case 0x1A:					//T=1
							len = (cmdbuf[12] << 8) | cmdbuf[13];
							len &= 0xFFF;
							//len = tk_usb_command_handler(uctx, cmdbuf + i + 3, len, resbuf + i + 3);
							if(uctx->handler != NULL) len = uctx->handler(uctx->cardCtx, cmdbuf + i + 3, len, resbuf + i + 3);
							else len = 0;
							resbuf[12] = (len >> 8) | (cmdbuf[12] & 0x40);
							resbuf[13] = len & 0xFF;
							i += 3;
							resbuf[i + len] = if_usb_checksum(resbuf + 12, len + 2);			//checksum??
							i++;
							break;
						default:
						case 0x06:				//configuration??
							len = 2;
							i += 4;
							resbuf[i] = 0x6D;
							resbuf[i+1] = 0x00;
							break;
						case 0x1B:					//T=0
							len = (cmdbuf[12] << 8) | cmdbuf[11];
							//len = tk_usb_command_handler(uctx, cmdbuf + i + 4, len, resbuf + i + 4);
							if(uctx->handler != NULL) len = uctx->handler(uctx->cardCtx, cmdbuf + i + 4, len, resbuf + i + 4);
							else len = 0;
							resbuf[12] = len >> 8;
							resbuf[11] = len & 0xFF;
							i += 4;
							break;
					}
					if(len == 0) return;
					i += len;
					resbuf[1] = i - 10;
				}
				break;
			case 0x62:			//get ATR (reset)
				if(uctx->cardCtx == NULL) break;
				resbuf[0] = 0x80;
				resbuf[1] = uctx->cardCtx->atrlen;
				memcpy(resbuf + 10, uctx->cardCtx->atr, uctx->cardCtx->atrlen);
				break;
			case 0x61:			//change configuration (PPS)
				resbuf[0] = 0x82;
				resbuf[1] = cmdbuf[1];
				resbuf[9] = uctx->cardCtx->protocol & 0x0F;
				memcpy(resbuf + 10, cmdbuf + 10, resbuf[1]);
				break;
			case 0x6C:			//get current configuration
				resbuf[0] = 0x82;
				resbuf[1] = 0x07;
				resbuf[9] = uctx->cardCtx->protocol & 0x0F;
				resbuf[10] = uctx->cardCtx->fidi;
				resbuf[11] = 0x10;		//??
				resbuf[13] = 0x45;		//??
				resbuf[15] = 0x80;		//??
				break;
			case 0x65:			//get card inserted
				uctx->status(uctx, if_card_state(uctx->cardCtx));
			case 0x63:			//disconnect card		(???)
			case 0x71:			//disconnect card		(???)
				resbuf[0] = 0x81;
				resbuf[7] = 0x01;
				resbuf[9] = 0x01;
				break;
		}
		uctx->write(uctx, resbuf, resbuf[1] + 10);
	}
}
#endif


#if (SHARD_USB_DRIVER == SHARD_DRIVER_KRON)
#if 0
void if_usb_handler(usb_context_p uctx, uint32 event) {
	//called when usb receiving command
	uint8 cmdbuf[258];		//beware of buffer overflow attack
	uint8 resbuf[258];			//beware of buffer overflow attack
	uint16 len;
	uint16 i;
	uint8 c;
	memset(resbuf, 0, sizeof(resbuf));
	//if(event == USB_EVT_OUT) {
	{
		len = uctx->read(uctx, cmdbuf);
		//memcpy(resbuf + 2, cmdbuf + 2, 5);		//copy sequence number
		if(len > 258) return;		//invalid length
		len = uctx->handler(uctx->shandle, cmdbuf, len, resbuf);
		if(len != 0) {
			uctx->write(uctx, resbuf, len);
		}
	}
}
#endif
#endif

#if SHARD_RTOS_ENABLED
static void if_usb_task() {
	os_message * msg;
	usb_context_p ctx = os_get_context();
	uint32 psw;	//uint16 ret16;
	usb_msg_param * params;
	uint8 buffer[256];
	uint8 buflen = 8;
	USBD_CUSTOM_HID_HandleTypeDef *hhid;
	//struct net_async_params * params;
	while(1) {
		msg = os_dequeue_message();
		if(msg != NULL) {
			ctx = msg->context;
			if(ctx == NULL) goto abort_operation;
			if(msg->reqlen == 0) goto abort_operation;
			params = msg->request;
			hhid = (USBD_CUSTOM_HID_HandleTypeDef*)ctx->instance->pClassData;
			check_again:
			if(params->endpoint == 0x81) {
				buflen = params->msg_len;
				memcpy(buffer, params->message, buflen);
				os_dispatch_reply(msg);
				os_delete_message(msg);
				goto send_report;
			}
			if (ctx->instance->dev_state == USBD_STATE_CONFIGURED )
			{
				if(hhid->state == CUSTOM_HID_IDLE)
				{
					hhid->state = CUSTOM_HID_BUSY;
					USBD_LL_Transmit(ctx->instance, params->endpoint, params->message, params->msg_len);
				}
				else {
					os_wait(100);
					goto check_again;
				}
			}
			abort_operation:
			os_dispatch_reply(msg);
			os_delete_message(msg);
		} else {
			//os_suspend();
			memset(buffer, 0, sizeof(buffer));
			buflen = 2;
			//USBD_LL_Transmit(ctx->instance, params->endpoint, buffer, 64);
			os_wait(100);
			///continue;
			send_report:
			hhid = (USBD_CUSTOM_HID_HandleTypeDef*)(ctx->instance)->pClassData;
			if (ctx->instance->dev_state == USBD_STATE_CONFIGURED ) {
				while(hhid->state  != CUSTOM_HID_IDLE) os_wait(100);
				USBD_CUSTOM_HID_SendReport(ctx->instance, buffer, buflen);
			}
			//os_wait(20);
		}
	}
}

//extern int16 g_audio_buffer[];
extern int16 g_audio_buffer[AUDIO_SAMPLING_FREQ] ;
extern int16 g_usb_audio_buffer[48312 * 3];
extern uint32 g_audio_buffer_tail;
static void if_usb_task2() {
	os_message * msg;
	usb_context_p ctx = os_get_context();
	uint32 psw;	//uint16 ret16;
	usb_msg_param * params;
	uint8 buffer[256];
	uint8 buflen = 8;
	USBD_CUSTOM_HID_HandleTypeDef *hhid;
	int prev_pos = 0;
	//struct net_async_params * params;
	while(1) {
		msg = os_dequeue_message();
		if(msg != NULL) {
			ctx = msg->context;
			if(ctx == NULL) goto abort_operation;
			if(msg->reqlen == 0) goto abort_operation;
			params = msg->request;
			
			if(((USBD_CUSTOM_HID_HandleTypeDef *)hUsbDeviceFS.pClassData)->AltSetting == 4) {
				
			}
			abort_operation:
			os_dispatch_reply(msg);
			os_delete_message(msg);
		}
		uint8 alt_setting = ((USBD_CUSTOM_HID_HandleTypeDef *)hUsbDeviceFS.pClassData)->AltSetting;
		//os_tick_callback_counter = (os_tick_callback_counter & 3);
		if(alt_setting == 4) {
			//if(os_tick_callback_counter == 0) {
			if(prev_pos != g_audio_buffer_tail) {
				g_audio_buffer_tail = (g_audio_buffer_tail % sizeof(g_usb_audio_buffer));
				//USBD_LL_Transmit(&hUsbDeviceFS, 0x81, (uint8 *)g_usb_audio_buffer + prev_pos , 1440);
				//g_audio_buffer_tail += 1440;
				prev_pos = g_audio_buffer_tail;
			}
		}
		//os_suspend();
		os_wait(30);
	}
}
#endif

void if_usb_init(usb_context_p uctx, void * shandle, uint16 (* handler)(void *, uint8 *, uint16, uint8 *)) {
	GPIO_InitTypeDef GPIO_InitStructure;    
	if(uctx != NULL) { 
		g_curUsbCtx = uctx;
		uctx->instance = &hUsbDeviceFS;
		uctx->shandle = shandle;
		uctx->handler = handler;
		uctx->read = if_usb_read;
		uctx->write = if_usb_write;
		uctx->status = if_usb_status;  
		uctx->debug = if_usb_debug;
		/* Control USB connecting via SW */
#if !defined(STM32F746xx)
		__HAL_RCC_GPIOD_CLK_ENABLE();
		GPIO_InitStructure.Pin = GPIO_PIN_8;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;	 		//复用开漏输出模式
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;		//输出畜频率为50MHz
		HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);
#endif
		//__HAL_RCC_USB_OTG_HS_CLK_ENABLE();
	  //__HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();
		//__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
		
		MX_USB_DEVICE_Init(uctx);
		//__HAL_USB_OTG_FS_WAKEUP_EXTI_ENABLE_IT();
		//__HAL_USB_OTG_FS_WAKEUP_EXTI_ENABLE_RISING_EDGE();
		//NVIC_EnableIRQ(OTG_FS_WKUP_IRQn);
		//NVIC_EnableIRQ(OTG_FS_IRQn);
		
		//__HAL_USB_HS_EXTI_ENABLE_IT();
		//__HAL_USB_OTG_HS_WAKEUP_EXTI_ENABLE_IT();
		//__HAL_USB_OTG_HS_WAKEUP_EXTI_ENABLE_RISING_EDGE();
		//NVIC_EnableIRQ(OTG_HS_WKUP_IRQn);
		//NVIC_EnableIRQ(OTG_HS_IRQn);
		//NVIC_EnableIRQ(OTG_HS_EP1_IN_IRQn);
		//NVIC_EnableIRQ(OTG_HS_EP1_OUT_IRQn);
#if SHARD_RTOS_ENABLED
		//os_create_task(uctx, if_usb_task2, "usb", 14, 2048);
#endif
		
		//__HAL_RCC_TIM11_CLK_ENABLE();
		//MX_TIM11_Init();
		//NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
	}
	//USB_Connect(TRUE);
}

void OTG_FS_WKUP_IRQHandler() {
	__HAL_USB_OTG_FS_WAKEUP_EXTI_CLEAR_FLAG();
}

void OTG_HS_WKUP_IRQHandler() {
	__HAL_USB_OTG_HS_WAKEUP_EXTI_CLEAR_FLAG();
}



/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 15;
  htim11.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim11.Init.Period = 10000;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim11, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
	
  /* USER CODE BEGIN TIM6_Init 2 */
	//HAL_TIM_Base_Start(&htim11);
	HAL_TIM_Base_Start_IT(&htim11);
	//HAL_TIMEx_PWMN_Start(&htim11);, 0);
	//HAL_TIMEx_PWMN_Start_IT(&htim11, 0);
  /* USER CODE END TIM6_Init 2 */

}