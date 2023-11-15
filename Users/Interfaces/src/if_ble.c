#include "defs.h"
#include "config.h"
#include "if_apis.h"
#include <string.h>
#include "if_ble.h"

#if SHARD_BLE_MODULE == MOD_ESP32
#include "if_net.h"


#endif


#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif

#include "..\..\toolkit\inc\tk_apis.h"


uint8 g_bt_buffer[1024];
uint16 g_bt_index = 0;
UART_HandleTypeDef * g_bt_handle = NULL;
static void dummy_handler(void) {
	
}

static void (*rx_done_handler)(void) = &dummy_handler;
static void (*tx_done_handler)(void) = &dummy_handler;


void USART3_IRQHandler(void)
{
	uint8 c;
	if(g_bt_handle == NULL) return;
#ifdef STM32F7
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)g_bt_handle, USART_FLAG_PE)) {
		__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)g_bt_handle, USART_CLEAR_PEF);
		return;
	}
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)g_bt_handle, USART_FLAG_RXNE) != RESET)
	{ 	
	    if(g_bt_index < sizeof(g_bt_buffer)) {
			g_bt_buffer[g_bt_index++] = HAL_USART_READ(g_bt_handle[0]);	//USART3->DR;
		} else {
			g_bt_index = 0;
		}
	}
#endif	
#ifdef STM32F4
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)g_bt_handle, USART_FLAG_PE)) {
		__HAL_USART_CLEAR_FLAG((USART_HandleTypeDef *)g_bt_handle, USART_FLAG_PE);
		return;
	}
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)g_bt_handle, USART_FLAG_RXNE) != RESET)
	{ 	
	    if(g_bt_index < sizeof(g_bt_buffer)) {
			g_bt_buffer[g_bt_index++] = HAL_USART_READ(g_bt_handle[0]);	//USART3->DR;
		} else {
			g_bt_index = 0;
		}
	}
#endif
}

static void if_ble_sendbyte(comm_context_p ctx, uint8 b) {
   	//while (!(ctx->handle.Instance->SR & USART_FLAG_TXE));
	while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx->handle.Instance, USART_FLAG_TXE) == RESET);
   	//ctx->handle.Instance->DR = b;
	HAL_USART_WRITE(ctx->handle, b);
}

static void if_ble_sendbytes(bt_context_p ctx, uint8 * buffer, uint16 length) {
	uint16 i;
	for(i=0;i<length;i++) if_ble_sendbyte((comm_context_p)ctx, buffer[i]);
}

void hal_uart_dma_set_sleep(uint8_t sleep){
/*
	// RTS is on PD12 - manually set it during sleep
	GPIO_InitTypeDef RTS_InitStruct;
	RTS_InitStruct.Pin = GPIO_PIN_12;
    RTS_InitStruct.Pull = GPIO_NOPULL;
    RTS_InitStruct.Alternate = GPIO_AF7_USART3;
	if (sleep){
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
		RTS_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	    RTS_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	} else {
		RTS_InitStruct.Mode = GPIO_MODE_AF_PP;
	    RTS_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	}

	HAL_GPIO_Init(GPIOD, &RTS_InitStruct);
*/
//	if (sleep){
//		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
//	}
	//hal_uart_needed_during_sleep = !sleep;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if (huart == g_bt_handle){
		(*tx_done_handler)();
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart == g_bt_handle){
		(*rx_done_handler)();
	}
}

void hal_uart_dma_init(void){
	//bluetooth_power_cycle();
}
void hal_uart_dma_set_block_received( void (*the_block_handler)(void)){
    rx_done_handler = the_block_handler;
}

void hal_uart_dma_set_block_sent( void (*the_block_handler)(void)){
    tx_done_handler = the_block_handler;
}

void hal_uart_dma_set_csr_irq_handler( void (*the_irq_handler)(void)){

	GPIO_InitTypeDef CTS_InitStruct = {
		.Pin       = GPIO_PIN_11,
		.Mode      = GPIO_MODE_AF_PP,
		.Pull      = GPIO_PULLUP,
		.Speed     = GPIO_SPEED_FREQ_VERY_HIGH,
		.Alternate = GPIO_AF7_USART3,
	};
    //cts_irq_handler = the_irq_handler;
}

int  hal_uart_dma_set_baud(uint32_t baud){
	//huart3.Init.BaudRate = baud;
	//g_bt_handle->
	//HAL_UART_Init(&huart3);
	//g_bt_handle->Init.BaudRate = baud;
	return 0;
}

void hal_uart_dma_send_block(const uint8_t *data, uint16_t size){
	if_ble_sendbytes((bt_context_p)g_bt_handle, (uint8 *)data, size);
	if(tx_done_handler != NULL) tx_done_handler();		//call complete callback
}

void hal_uart_dma_receive_block(uint8_t *data, uint16_t size){
	uint16 timeout=200;
	uint32 tickstart = HAL_GetTick();
	g_bt_index = 0;
	NVIC_EnableIRQ(USART3_IRQn);
	while(g_bt_index < size && HAL_GetTick() < (tickstart + timeout) ) {
		if_delay(40);
	}
	NVIC_DisableIRQ(USART3_IRQn);
	if(g_bt_index != 0) {
		memcpy(data, g_bt_buffer, size);
		if(rx_done_handler != NULL) rx_done_handler();		//call complete callback
	}
}


#if SHARD_BLE_MODULE == MOD_HM11
#include "btstack_config.h"
#include "hci_cmd.h"
#include "btstack.h"
#include "btstack_run_loop_embedded.h"
#include "btstack_uart_block.h"
#include "classic/btstack_link_key_db_memory.h"


static void if_ble_sendstring(bt_context_p ctx, uint8_t * buffer) {
	uint16 len = strlen((char *)buffer);
	uint16 i;
	for(i=0;i<len;i++){
		if_ble_sendbyte((comm_context_p)ctx, buffer[i]);
	}
}

static uint16 if_ble_command_s(bt_context_p ctx, uint8_t * command, uint8 * response, uint32 timeout, uint32 extra_timeout) {
	uint32 i;
	uint8 c;
	uint16 len =0;
	uint16 state = 0;
	uint32 tickstart;
	if(command != NULL) {
		if_ble_sendstring(ctx, command);
	}
	tickstart = HAL_GetTick();
	g_bt_index = 0;
#if SHARD_RTOS_ENABLED
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TC);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_PE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_ERR);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_IDLE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TXE);
	__HAL_USART_ENABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_RXNE);
	__HAL_UART_CLEAR_PEFLAG((UART_HandleTypeDef *)ctx);
	if(response == NULL) goto exit_send_command;
	NVIC_EnableIRQ(USART3_IRQn);
	while((HAL_GetTick() - tickstart) < timeout && g_bt_index == 0) { if_delay(40); }
	
	while((HAL_GetTick() - tickstart) < timeout) {
		if(len == g_bt_index) break;				//no new data received
		else {
			len = g_bt_index;
			timeout += extra_timeout;
			if_delay(extra_timeout);						//keep waiting for another extra 200ms
		}
	}
	len = g_bt_index;
	memcpy(response, g_bt_buffer, len);
	NVIC_DisableIRQ(USART3_IRQn);
	g_bt_index = 0;
#else
	while((HAL_GetTick() - tickstart) < timeout) { if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) break; }
	if((HAL_GetTick() - tickstart) >= timeout) goto exit_send_command;
	for(i=0;i<200000;i++) {  
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
			response[len++] = ctx->handle.Instance->DR;
			i = 0;
		}
	}
#endif
	exit_send_command:
	return len;
}

static uint16 if_ble_command(bt_context_p ctx, uint8_t * command, uint8 * response, uint32 timeout) {
	return  if_ble_command_s(ctx, command, response, timeout, 40);
}

uint8 if_comm_init(comm_context * ctx) {
	GPIO_InitTypeDef GPIO_InitStructure;         
	UART_HandleTypeDef usartHandle;
	uint16 baud = 9600, parity = 0, stopbit = 0;
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART3_CLK_ENABLE();
	//icc clock
	/* USART3 Tx (PD8) */
	GPIO_InitStructure.Pin = GPIO_PIN_8;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;	 					//default AF_PP
	GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
	GPIO_InitStructure.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* USART3 Rx (PD9)  */
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;						//default AF_PP
	GPIO_InitStructure.Pull = GPIO_PULLUP;								//default PULLUP
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
	GPIO_InitStructure.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);	
	//RTS
	GPIO_InitStructure.Pin = GPIO_PIN_12;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;						//default AF_PP
	GPIO_InitStructure.Pull = GPIO_PULLUP;								//default PULLUP
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
	GPIO_InitStructure.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);	
	//CTS
	GPIO_InitStructure.Pin = GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;						//default AF_PP
	GPIO_InitStructure.Pull = GPIO_PULLUP;								//default PULLUP
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
	GPIO_InitStructure.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);	
	
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
	usartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS;
	usartHandle.Init.OverSampling = UART_OVERSAMPLING_16;	//UART_OVERSAMPLING_16;
	//USART_InitStructure.HardwareFlowControl = USART_HardwareFlowControl_None;
	usartHandle.Init.Mode = USART_MODE_TX_RX;
	HAL_UART_Init((UART_HandleTypeDef *)&usartHandle);
	ctx->tx = GPIO_PIN_8;
	ctx->rx = GPIO_PIN_9;
	ctx->port = GPIOD;
	ctx->baudrate = 9600;
	ctx->handle.Instance = USART3;
	memcpy(ctx, &usartHandle, sizeof(UART_HandleTypeDef));
	g_bt_handle = (UART_HandleTypeDef *)ctx;
	return 0;
}

static const hci_transport_config_uart_t hci_config = {
	HCI_TRANSPORT_CONFIG_UART,
    9600,
    4000000,
    1,
    NULL
};


uint8 g_btstack_task_stack[2048];
static void ble_task_loop(void * param) {
	while(1) {
        btstack_run_loop_embedded_execute_once();
		if_delay(200);
	}
}

uint8 if_ble_init(bt_context * ctx) {
	OS_DEBUG_ENTRY(if_ble_init);
	uint8 buffer[64];
	uint8 ret = -1;
	const hci_transport_t * transport;
	const btstack_uart_block_t * uart_driver ;
	const btstack_link_key_db_t * btstack_link_key_db ;
	if((ctx->state & BLE_STATE_INITIALIZED) == 0) {
		if(if_comm_init((comm_context_p)ctx) == 0) {
			ctx->dev_list = NULL;
			memset(ctx->host_mac, 0, 6);
			memset(ctx->slave_mac, 0, 6);
			if_ble_command(ctx, (uint8 *)"AT", buffer, 500) ;		//disconnect any previous connection
			if(if_ble_command(ctx, (uint8 *)"AT+ROLE1", buffer, 500) != 0) {
				if_ble_command(ctx, (uint8 *)"AT+IMME1", buffer, 500) ;
				if_ble_command(ctx, (uint8 *)"AT+MODE0", buffer, 500) ;
				if_ble_command(ctx, (uint8 *)"AT+NOTI1", buffer, 500) ;
				if_ble_command(ctx, (uint8 *)"AT+NAMECardinal", buffer, 500) ;
				ret = if_ble_get_mac(ctx);
				ctx->state |= BLE_STATE_INITIALIZED;
				//initialize btstack library
				btstack_memory_init();
				btstack_run_loop_init(btstack_run_loop_embedded_get_instance());
				//init HCI
				uart_driver = btstack_uart_block_embedded_instance();
				transport = hci_transport_h4_instance(uart_driver);
				hci_init(hci_transport_h4_instance(btstack_uart_block_embedded_instance()), (void*) &hci_config);
				// setup Link Key DB using TLV
				btstack_link_key_db = btstack_link_key_db_memory_instance();
				hci_set_link_key_db(btstack_link_key_db);
				//hci_set_chipset(btstack_chipset_cc256x_instance());
				os_create_task_static(ctx, ble_task_loop, "bt", 90, (lp_void)g_btstack_task_stack, sizeof(g_btstack_task_stack));
			}
		}
	}
	OS_DEBUG_EXIT();
	return ret;
}

uint8 if_ble_get_mac(bt_context_p ctx) {
	OS_DEBUG_ENTRY(if_ble_get_mac);
	uint8 buffer[64];
	uint8 len;
	char * ptr;
	uint8 ret = -1;
	memset(buffer, 0, sizeof(buffer));
	len = if_ble_command_s(ctx, (uint8 *)"AT+ADDR?", buffer, 200, 50);
	if((ptr = strstr((char *)buffer, "OK+ADDR:")) != NULL) {
		tk_hex2bin((uint8 *)ptr + 8, ctx->host_mac);
		ret = 0;
	}
	OS_DEBUG_EXIT();
	return ret;
}

static bt_device_p if_bt_device_create(bt_context_p ctx, uint8 * mac) {
	OS_DEBUG_ENTRY(if_bt_device_create);
	uint8 i = 0;
	uint8 dev_macs[64];
	uint8 state = 0;
	uint8 parsing = 1;
	uint8 bufidx = 0;
	bt_device_p dev = (bt_device_p)malloc(sizeof(bt_device));
	memset(dev_macs, 0, sizeof(dev_macs));
	if(dev != NULL) {
		memcpy(dev_macs, mac, 12);
		tk_hex2bin(dev_macs, dev->mac);
		sprintf((char *)dev->name, "%02x:%02x:%02x:%02x:%02x:%02x", dev->mac[0], dev->mac[1], dev->mac[2], dev->mac[3], dev->mac[4], dev->mac[5]);
		dev->next = NULL;
		dev->ctx = ctx;
	}
	OS_DEBUG_EXIT();
	return dev;
}

void if_bt_device_clear(bt_device_p root) {
	OS_DEBUG_ENTRY(if_bt_device_clear);
	bt_device_p tlist = root;
	bt_device_p plist;
	while(tlist != NULL) {
		plist = tlist;
		tlist = tlist->next;
		memset(plist, 0, sizeof(net_ssid));
		free(plist);
	}
	OS_DEBUG_EXIT();
}

uint8 if_ble_dev_list(bt_context * ctx) {
	OS_DEBUG_ENTRY(if_ble_dev_list);
	uint8 buffer[256];
	uint8 i = 0;
	bt_device_p iterator = NULL;
	uint8 dev_count = 0;
	uint8 len;
	if(ctx->state & BLE_STATE_CONNECTED) {
		iterator = ctx->dev_list;
		while(iterator != NULL) {
			dev_count++;
			iterator = iterator->next;
		}
		goto exit_dev_list;
	}
	if_ble_command(ctx, (uint8 *)"AT", buffer, 500);		//wakeup command
	len = if_ble_command_s(ctx, (uint8 *)"AT+DISC?", buffer, 500, 1000);
	if(len != 0) { if_bt_device_clear(ctx->dev_list); ctx->dev_list = NULL; }
	iterator = ctx->dev_list;
	while(i < len) {
		if(memcmp(buffer + i, "OK+DISCS", 8) == 0) { i += 8; }
		if(memcmp(buffer + i, "OK+DISCE", 8) == 0) break;
		if(memcmp(buffer + i, "OK+DIS", 6) == 0) {
			dev_count ++;
			if(iterator == NULL) {
				iterator = if_bt_device_create(ctx, buffer + i + 8);		//mac address
				ctx->dev_list = iterator;
			} else {
				iterator->next = if_bt_device_create(ctx, buffer + i + 8);		//mac address
			}
			i += (8 + 12);
		} else 
			i++;
	}
	exit_dev_list:
	OS_DEBUG_EXIT();
	return dev_count;
}

uint8 if_ble_connect(bt_context * ctx, bt_device_p dev) {
	OS_DEBUG_ENTRY(if_ble_connect);
	uint8 buffer[64];
	uint8 len;
	uint8 ret = -1;
	uint8 hex_buffer[32];
	char * ptr;
	len = tk_bin2hex(dev->mac, 6, hex_buffer);
	hex_buffer[12] = 0;
	sprintf((char *)buffer, "AT+CON%s", hex_buffer);
	if_ble_disconnect(ctx);
	//if(ctx->state & IF_BLE_CONNECTED) goto exit_connect;	//already connected
	if(ctx->state & BLE_STATE_INITIALIZED) {
		len = if_ble_command_s(ctx, buffer, buffer, 1000, 1000);
		if((ptr = strstr((char *)buffer, "OK+CONNA")) != NULL) {
			//connection accepted
			if(strstr((char *)buffer, "OK+CONNE") != NULL) goto exit_connect;
			if(strstr((char *)buffer, "OK+CONNF") != NULL) goto exit_connect;
			ctx->state |= BLE_STATE_CONNECTED;
			memcpy(ctx->slave_mac, dev->mac, 6);
			ret = 0;
		}
	}
	exit_connect:
	OS_DEBUG_EXIT();
	return ret;
}


uint8 if_ble_disconnect(bt_context * ctx) {
	OS_DEBUG_ENTRY(if_ble_disconnect);
	uint8 buffer[64];
	uint8 len = 0;
	uint8 ret = -1;
	len = if_ble_command_s(ctx, (uint8 *)"AT", buffer, 500, 1000);
	if(strstr((char *)buffer, "OK") != NULL) { 
		ctx->state &= ~(BLE_STATE_CONNECTED | BLE_STATE_OPENED);		//close and disconnect
		ret = 0;
	}
	OS_DEBUG_EXIT();
	return ret;
}

static uint8_t  service_index = 0;
static uint8_t  channel_nr[10];
static void store_found_service(const char * name, uint8_t port){
    printf("APP: Service name: '%s', RFCOMM port %u\n", name, port);
    channel_nr[service_index] = port;
    //service_name[service_index] = (char*) malloc(SDP_SERVICE_NAME_LEN+1);
    //strncpy(service_name[service_index], (char*) name, SDP_SERVICE_NAME_LEN);
    //service_name[service_index][SDP_SERVICE_NAME_LEN] = 0;
    service_index++;
}

static void report_found_services(void){
    //printf("\n *** Client query response done. ");
    if (service_index == 0){
    //    printf("No service found.\n\n");
    } else {
    //    printf("Found following %d services:\n", service_index);
    }
    //int i;
    //for (i=0; i<service_index; i++){
    //    printf("     Service name %s, RFCOMM port %u\n", service_name[i], channel_nr[i]);
    //}    
    //printf(" ***\n\n");
}

static void handle_query_rfcomm_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    switch (packet[0]){
        case SDP_EVENT_QUERY_RFCOMM_SERVICE:
            store_found_service(sdp_event_query_rfcomm_service_get_name(packet), 
                                sdp_event_query_rfcomm_service_get_rfcomm_channel(packet));
            break;
        case SDP_EVENT_QUERY_COMPLETE:
            if (sdp_event_query_complete_get_status(packet)){
                //printf("SDP query failed 0x%02x\n", sdp_event_query_complete_get_status(packet));
                break;
            } 
            //printf("SDP query done.\n");
            report_found_services();
            break;
    }
}

//static uint16_t  rfcomm_mtu;
//static uint16_t  rfcomm_cid = 0;
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);
	bt_context_p ctx = (bt_context_p)g_bt_handle;
    bd_addr_t event_addr;
    uint8_t   rfcomm_channel_nr;
    uint32_t class_of_device;
    if (packet_type != HCI_EVENT_PACKET) return;
    uint8_t event = hci_event_packet_get_type(packet);

    switch (event) {
        case BTSTACK_EVENT_STATE:
            // BTstack activated, get started 
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                sdp_client_query_rfcomm_channel_and_name_for_uuid(&handle_query_rfcomm_event, ctx->slave_mac, BLUETOOTH_ATTRIBUTE_PUBLIC_BROWSE_ROOT);
            }
            break;
		case RFCOMM_EVENT_INCOMING_CONNECTION:
			// data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
			rfcomm_event_incoming_connection_get_bd_addr(packet, event_addr); 
			rfcomm_channel_nr = rfcomm_event_incoming_connection_get_server_channel(packet);
			ctx->cid = rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
			//printf("RFCOMM channel %u requested for %s\n", rfcomm_channel_nr, bd_addr_to_str(event_addr));
			rfcomm_accept_connection(ctx->cid);
			break;
			
		case RFCOMM_EVENT_CHANNEL_OPENED:
			// data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
			if (rfcomm_event_channel_opened_get_status(packet)) {
				//printf("RFCOMM channel open failed, status %u\n", rfcomm_event_channel_opened_get_status(packet));
			} else {
				ctx->cid = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
				ctx->mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
				//printf("RFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n", rfcomm_cid, rfcomm_mtu);
				//test_reset();

				// disable page/inquiry scan to get max performance
				gap_discoverable_control(0);
				gap_connectable_control(0);
			}
			break;

		case RFCOMM_EVENT_CHANNEL_CLOSED:
			//printf("RFCOMM channel closed\n");
			ctx->cid = 0;

			// re-enable page/inquiry scan again
			//gap_discoverable_control(1);
			//gap_connectable_control(1);
			break;
		case RFCOMM_EVENT_CAN_SEND_NOW:
			ctx->cid = rfcomm_event_can_send_now_get_rfcomm_cid(packet);
			rfcomm_send(ctx->cid, (uint8 *)"hello world\r\n", 13);
			rfcomm_request_can_send_now_event(ctx->cid);
			break;
		case SDP_EVENT_QUERY_RFCOMM_SERVICE:
            break;
        case SDP_EVENT_QUERY_COMPLETE:
            if (sdp_event_query_complete_get_status(packet)){
                //printf("SDP query failed 0x%02x\n", sdp_event_query_complete_get_status(packet));
                break;
            } 
            break;
		case HCI_EVENT_TRANSPORT_PACKET_SENT: break;
        case HCI_EVENT_COMMAND_COMPLETE: break;
			
		case HCI_EVENT_PACKET: break;
        case RFCOMM_DATA_PACKET:
			
			break;
        default:
            break;
    }
}

#define RFCOMM_SERVER_CHANNEL 1
static btstack_packet_callback_registration_t hci_event_callback_registration;
void if_btdev_init(bt_context * ctx) {
	// register for HCI events
	hci_event_callback_registration.callback = &packet_handler;
	hci_add_event_handler(&hci_event_callback_registration);

	// init L2CAP
	l2cap_init();

    rfcomm_init();
    rfcomm_register_service(packet_handler, RFCOMM_SERVER_CHANNEL, 0xffff);
    // init SDP
    //gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);
	// turn on!
	hci_power_control(HCI_POWER_ON);
	
}

uint8 if_ble_open(bt_context * ctx, uint16 mode) {
	//return 0 on success
	return rfcomm_create_channel(packet_handler, ctx->slave_mac, RFCOMM_SERVER_CHANNEL, NULL); 
}

uint8 if_ble_send(bt_context * ctx, uint8 * data, uint16 size, uint16 timeout) {
	uint16 len = 0;
	uint16 t = 0;
	if(ctx->cid == 0) return 0;
	while( rfcomm_can_send_packet_now(ctx->cid) == 0 && t<timeout) {
		t += 40;
		if_delay(40);
	}
	if( rfcomm_can_send_packet_now(ctx->cid) != 0) {
		if(rfcomm_send(ctx->cid, data, size) == 0) len = size;
	}
	return len;
}
uint8 if_ble_accept(bt_context * ctx, uint32 timeout);
uint16 if_ble_recv(bt_context * ctx, uint8 * response);

void if_ble_close(bt_context *ctx) {
	if(ctx->cid == 0) return;
	rfcomm_disconnect(ctx->cid);
	ctx->cid = 0;
}

#endif



