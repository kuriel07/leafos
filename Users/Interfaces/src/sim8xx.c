#include "defs.h"
#include "config.h"
#include "..\inc\if_apis.h"
#include "..\inc\if_net.h"
#include <string.h>
#include "if_net.h"
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#include "..\..\toolkit\inc\tk_apis.h"
#endif

#define ESP8266_TRANSPARENT_TRANSMISSION		0

uint8 gb_gsm_stat = 0;
net_conn * g_gsm_server_channel = NULL;

void sim_mac2bin(uint8* mac_str, uint8 * bin);

static uint8 net_hex2byte(uint8 hexchar) {
	if(hexchar >= 'a' && hexchar <= 'f') return (hexchar - 'a') + 10;
	if(hexchar >= 'A' && hexchar <= 'F') return (hexchar - 'A') + 10;
	if(hexchar >= '0' && hexchar <= '9') return hexchar - '0';
	return 0;
}

static uint8 ipstr2ip4(uint8 * str, uint8 ip[]) {
	uint8 i, j;
	uint8 cbuf[10];
	for(i=0;i<4;i++) {
		for(j=0;j<4;j++) {
			cbuf[j] = *str++;
			switch(cbuf[j]) {
				case 0:
				case '\"':
				case '.':
					cbuf[j] = 0;		//eos
					ip[3-i] = atoi((const char *)cbuf);
					j=4;
					break;
				default: break;
			}
		}
	}
}

#define ND_WAIT_STATUS			0
#define ND_WAIT_TIMEOUT			7
#define ND_WAIT_DATA			8
#define ND_CMD_END				9
#define ND_CMD_CONNECTED		11
#define ND_CMD_CLOSED			22
#define ND_WAIT_DATA_CHANNEL	12
#define ND_WAIT_DATA_LENGTH		13
#define ND_WAIT_DATA_IP			14
#define ND_WAIT_DATA_PORT		15
#define ND_CMD_GSM_CONNECTED 		27
#define ND_CMD_GSM_READY			28
#define ND_CMD_GSM_DISCONNECTED	29
#define ND_CMD_GSM_DISABLED			25
#define ND_CMD_GSM_ENABLED			26
#define ND_CMD_SEND_OK			31
#define ND_RES_SMS_RECV				40
#define ND_RES_SMS_INDEX			41

typedef struct net_def_state net_def_state;
struct net_def_state {
	uint8 length;
	const char * keyword;
	uint8 state;
};

const net_def_state g_sim_states[] = {
	{ 9, "+RECEIVE,", ND_WAIT_DATA_CHANNEL },
	{ 7, "SEND OK", ND_CMD_END },
	{ 2, "OK", ND_CMD_END },
	{ 5, "ERROR", ND_CMD_END },
	{ 7, "CONNECT", ND_CMD_CONNECTED },
	{ 6, "SERVER", ND_CMD_CONNECTED },
	{ 6, "CLOSED", ND_CMD_CLOSED },
	{ 12, "+CPIN: READY", ND_CMD_GSM_ENABLED },
	{ 16, "+CPIN: NOT READY", ND_CMD_GSM_DISABLED },
	{ 9, "SMS Ready", ND_CMD_GSM_READY },
	{ 11, "+PDP: DEACT", ND_CMD_GSM_DISABLED },
	{ 6, "+CMTI:", ND_RES_SMS_RECV },
	{ 0, NULL, ND_WAIT_STATUS },
};

//sms queue
uint8 g_sms_count = 0;
uint16 g_sms_queue[16];

static uint8 simcom_decode(net_context_p ctx) {
	static uint8 res_len = 0;
	static uint16 res_index = 0;
	static uint8 state = ND_WAIT_STATUS;
	static int16 cur_channel = -1;
	static uint16 sms_index;
	static uint8 temp[20];
	static uint8 remote_ip[4];
	static uint16 remote_port;
	static uint8 temp_index = 0;
	static uint16 data_length = 0;
	static uint8 lost_cntr = 100;
	uint8 temp_ip[4];
	net_conn * conn;
	uint32 start_tick;
	//uint8 total, i;
	//uint16 local_port;
	uint8 c;
	uint8 index = 0;
	if(net_buffer_dequeue(&c) != 0) return -1; 
	gb_gsm_stat |= NET_STAT_BUF_RDY;		//buffer ready
	switch(state) {
		case ND_CMD_GSM_DISCONNECTED:
			ctx->state &= ~(IF_NET_STATE_CONNECTED | IF_NET_STATE_IP_ASSIGNED);		//clear status
			goto wait_status;
		case ND_CMD_GSM_CONNECTED:
			ctx->state |= IF_NET_STATE_CONNECTED;
			goto wait_status;
		case ND_CMD_GSM_DISABLED:
			ctx->state |= IF_NET_STATE_DISABLED;
			ctx->state &= ~(IF_NET_STATE_CONNECTED | IF_NET_STATE_IP_ASSIGNED);		//clear status
			if_net_disable(ctx);
			goto wait_status;
		case ND_CMD_GSM_ENABLED	:
			ctx->state |= IF_NET_STATE_ENABLED;
			if_net_enable(ctx);
			goto wait_status;
		case ND_CMD_GSM_READY:
			ctx->state |= IF_NET_STATE_READY;
			//ctx->exception_handler(ctx, "gsm ready");
			goto wait_status;
		case ND_CMD_CLOSED:
			conn = net_conn_get_client(cur_channel);
			if(cur_channel != -1 && conn != NULL)
				conn->state = NET_CONN_CLOSED;
			goto wait_status;
		case ND_CMD_CONNECTED:
			conn = net_conn_get_client(cur_channel);
			if(cur_channel != -1 && conn != NULL)
				conn->state |= NET_CONN_CONNECTED;
			goto wait_status;
		case ND_RES_SMS_RECV:
			if(c == ',') state = ND_RES_SMS_INDEX;
			break;
		case ND_RES_SMS_INDEX:
			if(c == '\n') {
				temp[temp_index] = 0;
				sms_index = atoi((const char *)temp);
				temp_index = 0;
				g_sms_queue[g_sms_count++] = sms_index;
				//reset decode state
				state = ND_WAIT_STATUS;			
				gb_gsm_stat |= NET_STAT_BUF_RDY;		//buffer ready
			} else {
				temp[temp_index++] = c;
			}
			break;
		case ND_WAIT_STATUS:
			wait_status:
			if(c == '\n') { res_len = 0; res_index = net_buffer_tail(); }
			else if(res_len == 0) {
				if(c >= '0' && c <= '9') { 
					cur_channel = c - '0';
					if(cur_channel >= NET_MAX_CHANNEL) cur_channel = -1;
				}
				else if(c == ',') break;		//skip comma
				else res_len++;
			}
			else {
				res_len++;
				for(index =0;;index++) {
					if(g_sim_states[index].length == 0) break;		//end of matching states
					if(g_sim_states[index].length == res_len) {
						if(memcmp(g_sim_states[index].keyword, gba_net_bufstream + res_index, res_len) == 0) {
							//matching keyword
							if(strcmp(g_sim_states[index].keyword, "SEND OK") == 0) {
								temp_index = 0;
							}
							state = g_sim_states[index].state;
							temp_index = 0;
							break;
						}
					}
				}
			}
			break;
		case ND_CMD_END:
			if(c == '\n') {
		case ND_CMD_SEND_OK:
				state = ND_WAIT_STATUS;			//reset decode state
				gb_gsm_stat |= NET_STAT_BUF_RDY;		//buffer ready
				//net_buffer_reset();
			}
			break;
		case ND_WAIT_DATA_CHANNEL:
			if(c == ',') {
				temp[temp_index] = 0;
				cur_channel = atoi((const char *)temp);
				temp_index = 0;
				state = ND_WAIT_DATA_LENGTH;
			} else {
				temp[temp_index++] = c;
			}
			break;
		case ND_WAIT_DATA_LENGTH:
			//conn = g_channels[cur_channel];
			conn = net_conn_get_client(cur_channel);
			if(conn == NULL) conn = g_gsm_server_channel;		//unknown channel, use server channel
			if(c == ',') {
				temp[temp_index] = 0;
				data_length = atoi((const char *)temp);
				conn->base.bufend += data_length;
				temp_index = 0;
				state = ND_WAIT_DATA_IP;
			} else {
				temp[temp_index++] = c;
			}
			break;
		case ND_WAIT_DATA_IP:
			conn = net_conn_get_client(cur_channel);
			if(conn == NULL) conn = g_gsm_server_channel;		//unknown channel, use server channel
			//conn = g_channels[cur_channel];
		if(c == ':') {
				temp[temp_index] = 0;
				//data_length = atoi((const char *)temp);
				ipstr2ip4(temp, (uint8 *)temp_ip);
				//check if new data exist, but previous data hasn't been processed
				start_tick = if_sys_tick();
				while((conn->state & NET_CONN_BUFRDY) && 
					memcmp(&conn->remote_ip, temp_ip, 4) != 0) {
					os_wait(40);		//data exist in the buffer
					if(if_sys_tick() > (start_tick + 5000)) {		//purge data after 5 seconds
						conn->base.bufend = data_length;
						conn->base.buflen = 0;
						conn->state &= ~NET_CONN_BUFRDY;
						break;
					}
				}
				memcpy(remote_ip, temp_ip, 4);
				temp_index = 0;
				state = ND_WAIT_DATA_PORT;
			} else {
				temp[temp_index++] = c;
			}
			break;
		case ND_WAIT_DATA_PORT:
			//conn = g_channels[cur_channel];
			if(c == '\n') {
				temp[temp_index] = 0;
				remote_port = atoi((const char *)temp);
				temp_index = 0;
				state = ND_WAIT_DATA;
			} else {
				temp[temp_index++] = c;
			}
			break;
		case ND_WAIT_DATA:
			conn = net_conn_get_client(cur_channel);
			if(conn == NULL) conn = g_gsm_server_channel;		//unknown channel, use server channel
			if(conn != NULL) {
				if(conn->base.buflen < conn->base.bufsz) {
					conn->state |= NET_CONN_BUSY;
					//put data in the buffer, increment buffer length
					if(conn->base.buffer != NULL && (conn->state & NET_CONN_READY) != 0) {
						conn->base.buffer[conn->base.buflen++] = c;
					}
					if(conn->base.buflen == conn->base.bufend) goto end_data_transfer;
				} else {
					end_data_transfer:
					conn->id = cur_channel;
					conn->state &= ~NET_CONN_BUSY;
					conn->state |= NET_CONN_BUFRDY;
					memcpy(&conn->remote_ip, remote_ip, 4);
					conn->remote_port = remote_port;
					data_length = 0;
					//gb_gsm_stat |= NET_STAT_BUF_RDY;		//buffer ready
					state = ND_WAIT_STATUS;
					res_len = res_index;		//reset result_len back to last response_status
					res_index = net_buffer_tail();
					cur_channel = -1;			//reset current channel
				}
			}
			break;
	}
	return 0;
}

void simcom_set_buffer(net_conn_p conn, uint8 * buffer, uint16 bufsz) {
	if(conn->base.buf_release != NULL) conn->base.buf_release(conn->base.buffer);
	conn->base.buffer = buffer;
	conn->base.bufsz = bufsz;
	conn->base.buf_release = NULL;
}

void if_simcom_task(void) {
	//task for handling response from wifi-module
	net_context_p ctx = os_get_context();
	while(1) {
		while(simcom_decode(ctx) == 0);
		os_wait(40);
	}
}

void simcom_reset(net_context_p ctx) {
	//no reset
	//turning off device
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3,GPIO_PIN_SET);
	if_delay(40);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	if_delay(800);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
	//turning on device
	if_delay(2000);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3,GPIO_PIN_SET);
	if_delay(40);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	if_delay(1200);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
}

void sim8xx_init_config(net_context_p ctx) {
	GPIO_InitTypeDef GPIO_InitStructure;             
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
#ifdef STM32F7
	__HAL_RCC_USART1_CONFIG(RCC_USART1CLKSOURCE_PCLK2);
#endif
	__HAL_RCC_USART1_CLK_ENABLE();
	//icc clock
   	/* USART1 Tx (PA9) */
  	GPIO_InitStructure.Pin = GPIO_PIN_9;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;	 					//default AF_PP
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;		//default HIGH
	GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIO_PinAFConfig(GPIOA, GPIO_PIN_9, GPIO_AF_USART1);
  	/* USART1 Rx (PA10)  */
  	GPIO_InitStructure.Pin = GPIO_PIN_10;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;						//default AF_PP
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;								//default PULLUP
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;		//default HIGH
	//GPIO_SetBits(GPIOA, GPIO_PIN_10);
	GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);	
	//reset
	GPIO_InitStructure.Pin = GPIO_PIN_8;	//reset
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)	
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;		
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);		
	//chip enable
	GPIO_InitStructure.Pin = GPIO_PIN_11;						//chip-en
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)			
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);		
	//chip enable
	GPIO_InitStructure.Pin = GPIO_PIN_1;						//chip-en
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)			
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);		
	//power enable
	GPIO_InitStructure.Pin = GPIO_PIN_3;						//chip-en
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)			
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);		
	//pwrkey
	GPIO_InitStructure.Pin = GPIO_PIN_3;						//chip-en
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//peripheral (alternate function push pull)			
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	ctx->rst = GPIO_PIN_8;
	ctx->cs = GPIO_PIN_11;
	ctx->port = GPIOA;
	ctx->baudrate = 115200;
	ctx->handle.Instance = USART1;
	ctx->handle.Init.BaudRate        	= ctx->baudrate; 
	ctx->handle.Init.WordLength 		= USART_WORDLENGTH_8B; 	//8位数据
	ctx->handle.Init.StopBits           = USART_STOPBITS_1;	 		//停止位1位
	ctx->handle.Init.Parity             = USART_PARITY_NONE ;	 	//无	
	ctx->handle.Init.HwFlowCtl 			= UART_HWCONTROL_NONE;
	ctx->handle.Init.OverSampling 		= UART_OVERSAMPLING_16;	//UART_OVERSAMPLING_16;
#ifdef STM32F7
	ctx->handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
#endif
	//USART_InitStructure.HardwareFlowControl = USART_HardwareFlowControl_None;
	ctx->handle.Init.Mode = USART_MODE_TX_RX;
	HAL_UART_Init((UART_HandleTypeDef *)ctx);
	//enable interrupt, since v2.0 interrupt is always enabled
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TC);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_PE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_ERR);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_IDLE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TXE);
#ifdef STM32F7
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_NE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_FE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_ORE);
#endif
	__HAL_USART_ENABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_RXNE);
	NVIC_EnableIRQ(USART1_IRQn);
	GPIO_ResetBits(GPIOA, GPIO_PIN_3);				//power-up device
	GPIO_SetBits(GPIOB, GPIO_PIN_1);				//enable device
	//simcom_reset(ctx);					//reset device
}

static void simcom_sendbyte(net_context_p ctx, uint8 b) {
	while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_TXE) == RESET);
   	//while (!(ctx->handle.Instance->SR & USART_FLAG_TXE));
   	//ctx->handle.Instance->DR = b;
	HAL_USART_WRITE(ctx->handle, b);
}

static void simcom_sendbytes(net_context_p ctx, uint8 * buffer, uint16 length) {
	uint16 i;
	for(i=0;i<length;i++) simcom_sendbyte(ctx, buffer[i]);
}

static void simcom_sendstring(net_context_p ctx, uint8_t * buffer) {
	uint16 len = strlen((char *)buffer);
	uint16 i;
	for(i=0;i<len;i++){
		simcom_sendbyte(ctx, buffer[i]);
	}
}

static uint16 simcom_compacting_net_buffer(net_context_p ctx, uint8 * response, uint16 length) {
	uint8 * nextptr;
	uint16 plen;
	uint8 * ptr;
	uint16 len = 0;
	uint16 i = 0;
	uint8 clen[10];
	uint8 ch_id = 0;
	while(len < length) {
		nextptr = response + len;
		try_next:						//start parsing response
		//nextptr[plen] = 0;			//set end of string for strstr
		if((ptr = (uint8 *)strstr((char *)nextptr, "+IPD,")) != NULL) {
			ptr +=5;
			i = 0;
			while(*ptr != 0) {
				clen[i++] = *ptr;
				if(*ptr == ',') {
					ch_id = clen[0] & 0x03;
					i = 0;
				}
				if(*ptr++ == ':') break;
			}
			clen[i] = 0;
			plen = atoi((const char *)clen);
			nextptr = ptr + plen;
#if NET_USE_CHANNEL
			//if(ch_id != ctx->ch_id) continue;		//invalid channel id, do not copy response
#endif
			memcpy(response + len, ptr, plen);
			len += plen;	
			if(plen < 1460) { //break;		//chunk not full, no next data
				break;
			}
			continue;
		}
		//changed to last operation (in case data already received), fixed 2016.3.31
		if(strstr((char *)nextptr, "SEND FAIL") != NULL) { len = 0; }
		if(strstr((char *)nextptr, "ERROR") != NULL) { len = 0; }				//link not valid
		if(strstr((char *)nextptr, "CLOSED") != NULL) {
			//set net_state to close (connection already closed)
			ctx->state &= ~IF_NET_STATE_OPENED;
		}
		break;
	}
	return len;
}

static uint16 simcom_command(net_context_p ctx, uint8_t * command, uint8 * response, uint32 timeout) {
	uint32 i;
	uint8 c;
	uint16 len =0;
	uint16 state = 0;
	uint32 tickstart;
	gb_gsm_stat = NET_STAT_CLEAR;
	net_buffer_reset();
	if(command != NULL) {
		simcom_sendstring(ctx, command);
		simcom_sendstring(ctx, (uint8 *)"\r\n");
		gb_gsm_stat = NET_STAT_CLEAR;
	}
#if SHARD_RTOS_ENABLED
	//gba_net_index = 0;
	if(response == NULL) goto exit_send_command;
	if(ctx->state & IF_NET_STATE_CRITICAL_SECTION) {
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) == RESET);
		for(i=0;i<20000;i++) {  
			if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
				//response[len++] = ctx->handle.Instance->DR;
				response[len++] = HAL_USART_READ(ctx->handle);
				i = 0;
			}
		}
	} else {
		tickstart = HAL_GetTick();
		while((HAL_GetTick() - tickstart) < timeout) { 
			if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) break;
		}
		if((HAL_GetTick() - tickstart) >= timeout) goto exit_send_command;
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TC);
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_PE);
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_ERR);
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_IDLE);
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TXE);
		__HAL_USART_ENABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_RXNE);
		__HAL_UART_CLEAR_PEFLAG((UART_HandleTypeDef *)ctx);
		//NVIC_EnableIRQ(USART1_IRQn);
		while((HAL_GetTick() - tickstart) < timeout && (gb_gsm_stat & NET_STAT_BUF_RDY) == 0);// os_wait(1);
		
		while((HAL_GetTick() - tickstart) < timeout) {
			if(len == net_buffer_length()) break;				//no new data received
			else {
				len = net_buffer_length();
				gb_gsm_stat &= ~NET_STAT_BUF_RDY;		//clear net ready
				timeout += 200;
				if_delay(200);						//keep waiting for another extra 200ms
			}
		}
		len = net_buffer_length();
		memcpy(response, gba_net_bufstream, len);
		//NVIC_DisableIRQ(USART1_IRQn);
		//gba_net_index = 0;
	}
#else
	for(i=0;i<200000;i++) {  
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
			response[len++] = ctx->handle.Instance->DR;
			i = 0;
		}
	}
#endif
	exit_send_command:
	net_buffer_reset();
	//__HAL_RCC_USART1_CLK_DISABLE();
	return len;
}

static uint16 simcom_data(net_context_p ctx, uint8_t * data, uint16 length, uint8 * response, uint32 timeout) {
	uint32 i;
	uint8 c;
	uint16 len =0;
	uint16 state = 0;
	uint32 tickstart;
	gb_gsm_stat = NET_STAT_CLEAR;
	//timeout *= 100000;
	tickstart = HAL_GetTick();
	//__HAL_RCC_USART1_CLK_ENABLE();
	net_buffer_reset();
	if(data != NULL) {
		for(i=0;i<length;i++) simcom_sendbyte(ctx, data[i]);
		while((HAL_GetTick() - tickstart) < timeout) { if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) break; }
		if((HAL_GetTick() - tickstart) >= timeout) goto exit_send_data;
	}
	//for(i=0;i<timeout;i++) { if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) break; }
	//if(i == timeout) goto exit_send_data;
#if SHARD_RTOS_ENABLED
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TC);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_PE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_ERR);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_IDLE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TXE);
	__HAL_USART_ENABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_RXNE);
	__HAL_UART_CLEAR_PEFLAG((UART_HandleTypeDef *)ctx);
	if(ctx->state & IF_NET_STATE_CRITICAL_SECTION) {
		//for(i=0;i<4000000 && gb_net_bufrdy == 0;i++);		//x seconds waiting till response
		//while(gb_net_bufrdy == 0);
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) == RESET);
		for(i=0;i<20000;i++) {  
			if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
				//response[len++] = ctx->handle.Instance->DR;
				response[len++] = HAL_USART_READ(ctx->handle);
				i = 0;
			}
		}
	} else {
		//NVIC_EnableIRQ(USART1_IRQn);
		while((HAL_GetTick() - tickstart) < timeout && (gb_gsm_stat & NET_STAT_BUF_RDY) == 0);// os_wait(1);
		
		while((HAL_GetTick() - tickstart) < timeout) {
			if(len == net_buffer_length()) break;				//no new data received
			else {
				len = net_buffer_length();
				gb_gsm_stat &= ~NET_STAT_BUF_RDY;		//clear net ready
				timeout += 1000;					//add another 200ms to timeout
				if_delay(1000);						//keep waiting for another extra 200ms
			}
		}
		len = net_buffer_length();
		memcpy(response, gba_net_bufstream, len);
		//NVIC_DisableIRQ(USART1_IRQn);
		//gba_net_index = 0;
		//net_buffer_reset();
	}
#else
	for(i=0;i<20000;i++) {  
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
			response[len++] = ctx->handle.Instance->DR;
			i = 0;
		}
	}
#endif
	exit_send_data:
	net_buffer_reset();
	//__HAL_RCC_USART1_CLK_DISABLE();
	return len;
}

static net_ssid_p simcom_ssid_create(net_context_p ctx, uint8 * buffer) {
	uint8 i = 0;
	uint8 state = 0;
	uint8 parsing = 1;
	net_ssid_p ssid = (net_ssid_p)malloc(sizeof(net_ssid));
	uint8 cbuf[256];
	uint8 bufidx = 0;
	if(ssid == NULL) return NULL;
	memset(cbuf, 0, sizeof(cbuf));
	memset(ssid, 0, sizeof(net_ssid));
	while(parsing) {
		switch(buffer[i]) {
			case ')':
				parsing = 0;
			case ',':
				cbuf[bufidx] = 0;
				switch(state) {
					case 0: 
						break;
					case 1:
						//SSID name
						if(bufidx > IF_NET_MAX_SSID_NAME) cbuf[IF_NET_MAX_SSID_NAME - 1] = 0;
						strncpy((char *)ssid->name, (const char *)cbuf, IF_NET_MAX_SSID_NAME);
						break;
					case 2:
						//attenuation
						ssid->att = atoi((const char *)cbuf);
						break;
					case 3:
						//mac address
						break;
					case 4:
						//security level
						ssid->sec = atoi((const char *)cbuf);
						break;
				}
				state ++;
				bufidx = 0;
				break;
			case '(':
			case '\"':
				break;
			default:
				cbuf[bufidx++] = buffer[i];
				break;
		}
		i++;
		if(i == 255) break;
	}
	ssid->netctx = ctx;
	return ssid;
}

void simcom_ssid_clear(net_ssid_p list) {
	net_ssid_p tlist = list;
	net_ssid_p plist;
	while(tlist != NULL) {
		plist = tlist;
		tlist = tlist->next;
		memset(plist, 0, sizeof(net_ssid));
		free(plist);
	}
}

net_ssid_p simcom_ssid_check(net_context_p ctx, uint8 * ssidname) {
	net_ssid_p iterator = ctx->ssid_list;
	if(ctx->ssid_list == NULL) return NULL;
	while(iterator != NULL) {
		if(strncmp((char *)iterator->name, (const char *)ssidname, IF_NET_MAX_SSID_NAME) == 0) return iterator;
		iterator = iterator->next;
	}
	return NULL;
}

void simcom_flush_sms(net_context_p ctx) {
	uint8 i, j;
	uint8 netcmd[64];
	uint8 netbuf[64];
	if(g_sms_count != 0) {
		for(i=0;i<g_sms_count;i++) {
			sprintf((char *)netcmd, "AT+CMGD=%d", g_sms_queue[i]);
			simcom_command(ctx, netcmd, netbuf, 200);
		}
		g_sms_count = 0;
	}
}

uint8 simcom_sleep(net_context_p ctx) {
	//sleep function automatically called after any net operation
	//flush sms if any
	simcom_flush_sms(ctx) ;
	//simcom_command(ctx, (uint8 *)"AT+SLEEP=1", NULL, 200);
}

uint8 simcom_get_ipconfig(net_context_p ctx) {
	uint16 len;
	uint8 * ptr;
	uint8 c;
	uint8 i, j;
	uint8 lbuf[32];
	uint8 net_buffer[1024];
	//get IP settings
	len = simcom_command(ctx, (uint8_t *)"AT+CIFSR", net_buffer, 400); 
	net_buffer[len] = 0;		//eos
	memset(ctx->ipv4, 0, 4);
	if((uint8 *)strstr((const char *)net_buffer, "ERROR") != 0) return FALSE;
	ptr = (uint8 *)strstr((const char *)net_buffer, "AT+CIFSR");
	if(ptr == NULL) return FALSE;
	ptr += 11;
	for(i=0;i<4;i++) {
		for(j=0;j<4;j++) {
			lbuf[j] = *ptr++;
			switch(lbuf[j]) {
				case '\n':
				case '\r':
				case '\"':
				case '.':
					lbuf[j] = 0;		//eos
					ctx->ipv4[i] = atoi((const char *)lbuf);
					j=4;
					break;
				default: break;
			}
		}
	}
	ctx->state |= IF_NET_STATE_IP_ASSIGNED;
	return memcmp(ctx->ipv4, "\x0\x0\x0\x0", 4) != 0;
}


uint8 simcom_get_status(net_context_p ctx, net_status_p stat) {
	uint16 len;
	char * ptr;
	uint8 status;
	uint8 index = 0;
	uint8 count = 0;
	uint8 link_id;
	uint8 remote_ip[4];
	uint16 remote_port;
	uint16 local_port;
	uint8 c;
	uint8 temp[30];
	len = simcom_command(ctx, (uint8_t *)"AT+CIPSTATUS", gba_net_bufstream, 200); 
	gba_net_bufstream[len] = 0;
	if((ptr = strstr((const char *)gba_net_bufstream, "STATUS:")) != NULL) {
		status = ptr[7] & 0x0F;
	}
	if(status == 3) {		//connected
		ptr = (char *)gba_net_bufstream;
		while((ptr = strstr((const char *)ptr, "+CIPSTATUS:")) != NULL) {
			ptr = ptr + 11;
			//link id
			index = 0;
			while((c = *ptr++) != ',') temp[index++] = c;
			temp[index] = 0;
			link_id = atoi((const char *)temp);
			//type
			index = 0;
			while((c = *ptr++) != ',') temp[index++] = c;
			temp[index] = 0;
			//remote ip
			index = 0;
			while((c = *ptr++) != ',') temp[index++] = c;
			temp[index] = 0;
			ipstr2ip4(temp, (uint8 *)remote_ip);
			//remote port
			index = 0;
			while((c = *ptr++) != ',') temp[index++] = c;
			temp[index] = 0;
			remote_port = atoi((const char *)temp);
			//local port
			index = 0;
			while((c = *ptr++) != ',') temp[index++] = c;
			temp[index] = 0;
			local_port = atoi((const char *)temp);
			//copy to stat
			stat[count].channel = link_id;
			memcpy(stat[count].remote_ip, remote_ip, 4);
			stat[count].remote_port = remote_port;
			stat[count].local_port = local_port;
			count++;
		}
	}
	return count;
}

uint8 simcom_is_connected(net_context_p ctx, uint8 * ssidname) {
	uint16 len;
	int16 att;
	uint8 * ptr;
	uint8 i=0,j,k;
	uint8 ret = -1;
	uint8 cbuf[20];
	uint8 n1, n2, n3;
	uint8 netcmd[64];
	uint8 netbuf[64];
	if(simcom_get_ipconfig(ctx)) ret = 0;
	if(ret == 0) {
		ctx->att = -36;		//dummy attenuation value
	}
	simcom_sleep(ctx);
	return ret;
}

uint16 simcom_wait_rdy(net_context_p ctx, uint32 timeout) {
	OS_DEBUG_ENTRY(simcom_wait_rdy);
	uint32 i;
	uint16 ret = -1;
	uint32 tickstart = HAL_GetTick();
	uint32 curtick;
	//enable interrupt (do not waste any received data)
	//NVIC_EnableIRQ(USART1_IRQn);
	gb_gsm_stat &= ~NET_STAT_BUF_RDY;		//buffer ready
	while(((curtick = HAL_GetTick()) - tickstart) < timeout) { 
		if( (gb_gsm_stat & NET_STAT_BUF_RDY) != 0) {
			ret = 0;
			goto exit_wait_rdy;
		}
	}
	//no data available, disable interrupt
	//NVIC_DisableIRQ(USART1_IRQn);
	exit_wait_rdy:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 simcom_ssid_list(net_context_p ctx) {
	//no SSID
	return 0;
}

uint8 simcom_apn_join(net_context_p ctx, uint8 * ssidname, uint8 * username, uint8 * password) {
	OS_DEBUG_ENTRY(simcom_apn_join);
	char cbuf[128];
	uint8 ret = 1;			//unable to connect (wrong password)
	uint16 len;
	uint8 * ptr;
	uint8 i, j;
	uint8 retry_cnt = 3;
	if(ctx->exception_handler != NULL) ctx->exception_handler(ctx, "connecting...");
	sprintf(cbuf, "AT+CSTT=\"%s\",\"%s\",\"%s\"", ssidname, username, password);
	//some delay here (please do something)
	ctx->state &= ~IF_NET_STATE_CONNECTED;
	while(1) {
		NET_LOCK(ctx);
		len = simcom_command(ctx, (uint8_t *)cbuf, gba_net_bufstream, 1000); 
		gba_net_bufstream[len] = 0;
		len = simcom_command(ctx, (uint8_t *)"AT+CIICR", gba_net_bufstream, 24000);
		gba_net_bufstream[len] = 0;
		if(len == 0) { //timeout
			if(retry_cnt == 0) break;
			retry_cnt--;
			NET_UNLOCK(ctx);
			if(ctx->exception_handler != NULL) ctx->exception_handler(ctx, "reconnecting...");
			simcom_init(ctx);		//re-init context
			continue;			//restart connection
		}
		if(strstr((char *)gba_net_bufstream, "ERROR") != NULL) {
			if(ctx->exception_handler != NULL) ctx->exception_handler(ctx, "unable to connect");
			break;
		}
		if(strstr((char *)gba_net_bufstream, "OK") != NULL) break;
	}
	NET_UNLOCK(ctx);
	if(simcom_get_ipconfig(ctx) ){
		//get ip configuration
		if(memcmp(ctx->ipv4, "\x0\x0\x0\x0", 4) == 0) {
			ret = 2;		//no IP assigned
		} else { 
			//successfully connected
			ret = 0;		//no error
			//push record
			ctx->fcnctr = 0;			//reset force connect counter on successfull connection
			if_net_ssidrec_push(ctx, ssidname, username, password);
		}
	}
	simcom_sleep(ctx);
	OS_DEBUG_EXIT();
	return ret;
}

uint16 simcom_escape_string(uint8 * payload, uint16 length, uint8 * escaped) {
	uint16 i=0,j=0;
	for(i=0;i<length;i++) {
		if(payload[i] == 0) {
			escaped[j++] = '\\';
			escaped[j++] = 0x30 + payload[i];
		} else {
			escaped[j++] = payload[i];
		}
	}
	return j;
}

net_conn_p simcom_sock_open(net_context_p ctx, uint8 * host, uint16 port, uint16 mode) {
	OS_DEBUG_ENTRY(simcom_sock_open);
	uint16 len;
	uint8 cbuf[500];
	uint8 ip4ddr[4];
	uint8 ipstr[20];
	net_conn_p conn;
	uint16 packet_size = 8192;
	uint16 timeout = 5000;
	uint8 retry = 3;
	if(mode & NET_TYPE_UDP) { 
		retry = 0;
		timeout = 3000;
		packet_size = 1460;
	} else {
		if((mode & NET_LISTEN) != 0) packet_size = 4096;
	}
	if(mode & NET_LISTEN && g_gsm_server_channel != NULL) goto exit_tcp_connect;		//server full
	use_system_dns:
	if(net_is_ip4ddr(host) != 0) {
		//try netbios
		if(netbios_translate(ctx, host, ip4ddr) == 0) goto use_ip_connect;
		goto use_hardware_dns;
		//try system DNS
		if(dns_translate(ctx, host, ip4ddr) != 0) goto use_hardware_dns;			//try translate domain name address to ip address
		use_ip_connect:
		sprintf((char *)ipstr, "%d.%d.%d.%d", ip4ddr[0], ip4ddr[1], ip4ddr[2], ip4ddr[3]);
		
		conn = net_conn_create(ctx, mode, timeout, port, NULL, packet_size, NULL);
		if(conn == NULL) goto exit_tcp_connect;		//no channel available

		if(mode & NET_TYPE_UDP) {
			if(mode & NET_LISTEN) {
#if NET_USE_CHANNEL
				sprintf((char *)cbuf, "AT+CIPSTART=%d,\"UDP\",\"%s\",0,%d,2", conn->id, ipstr, port);
#else
				sprintf((char *)cbuf, "AT+CIPSTART=\"UDP\",\"%s\",0,%d,2", ipstr, port);
#endif	
			} else {
#if NET_USE_CHANNEL
				sprintf((char *)cbuf, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d", conn->id, ipstr, port);
#else
				sprintf((char *)cbuf, "AT+CIPSTART=\"UDP\",\"%s\",%d", ipstr, port);
#endif
			}
		} else {
			if(mode & NET_LISTEN) {
				g_gsm_server_channel = conn;
				sprintf((char *)cbuf, "AT+CIPSERVER=1,%d", port);
			} else {
#if NET_USE_CHANNEL
				sprintf((char *)cbuf, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d", conn->id, ipstr, port);
#else
				sprintf((char *)cbuf, "AT+CIPSTART=\"TCP\",\"%s\",%d", ipstr, port);
#endif
			}
		}
	} else {
		use_hardware_dns:
		conn = net_conn_create(ctx, mode, timeout, port, NULL, packet_size, NULL);
		if(conn == NULL) goto exit_tcp_connect;		//no channel available
		if(mode & NET_TYPE_UDP) {
			if(mode & NET_LISTEN) {
#if NET_USE_CHANNEL
				sprintf((char *)cbuf, "AT+CIPSTART=%d,\"UDP\",\"%s\",0,%d,2", conn->id, host, port);
#else
				sprintf((char *)cbuf, "AT+CIPSTART=\"UDP\",\"%s\",0,%d,2", host, port);
#endif
			} else {
#if NET_USE_CHANNEL
				sprintf((char *)cbuf, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d", conn->id, host, port);
#else
				sprintf((char *)cbuf, "AT+CIPSTART=\"UDP\",\"%s\",%d", host, port);
#endif
			}
		} else {
			if(mode & NET_LISTEN) {
				g_gsm_server_channel = conn;
				sprintf((char *)cbuf, "AT+CIPSERVER=1,%d", port);
			} else {
#if NET_USE_CHANNEL
				sprintf((char *)cbuf, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d", conn->id, host, port);
#else
				sprintf((char *)cbuf, "AT+CIPSTART=\"TCP\",\"%s\",%d", host, port);
#endif
			}
		}
	}
	//wait for response OK
	len = simcom_command(ctx, (uint8 *)cbuf, gba_net_bufstream, timeout); 
	gba_net_bufstream[len] = 0;
	//wait additional response indicating connection status
	len += simcom_command(ctx, (uint8 *)NULL, gba_net_bufstream + len, 2000);
	gba_net_bufstream[len] = 0;
	if(strstr((char *)gba_net_bufstream, "ERROR") != NULL || 
		strstr((char *)gba_net_bufstream, "CONNECT FAIL") != NULL) {
		//error connecting to server
		retry_connection:
		if(conn != NULL) { 
			net_conn_close(conn);
			if(mode & NET_LISTEN) g_gsm_server_channel = conn;;	//failed to open server port
			conn = NULL;
		}
		if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->exception_handler != NULL) 
			ctx->exception_handler(ctx, "unable to connect");
		if(retry == 0) goto exit_tcp_connect;
		retry--;
		goto use_hardware_dns;
	}
	if(strstr((char *)gba_net_bufstream, "CONNECT OK") != NULL || 
		strstr((char *)gba_net_bufstream, "ALREADY CONNECT") != NULL) {
		//ready connect
		connect_ready:
		if((mode & NET_LISTEN) == 0)
			ctx->state |= IF_NET_STATE_OPENED;
	}
	exit_tcp_connect:
	OS_DEBUG_EXIT();
	return conn;
}

void simcom_sock_flush(net_conn_p conn) {
	char buffer[64];
	if(conn == NULL) return;
#if NET_USE_CHANNEL
	sprintf(buffer, "AT+CIPCLOSE=%d\r\n", conn->id);
#else
	sprintf(buffer, "AT+CIPCLOSE\r\n");
#endif
	simcom_sendstring(conn->netctx, (uint8 *)buffer);
}

void simcom_sock_close(net_conn_p conn) {
	OS_DEBUG_ENTRY(simcom_sock_close);
	uint32 cntr = 0;
	char buffer[64];
	if(conn == NULL) return;
	if(conn->state != NET_CONN_CLOSED) {
#if ESP8266_TRANSPARENT_TRANSMISSION
		simcom_sendstring(ctx, (uint8 *)"+++");
#endif
		if(conn->base.mode & NET_LISTEN) {
			sprintf(buffer, "AT+CIPSERVER=0,%d\r\n", conn->port);
			g_gsm_server_channel = NULL;			//release server channel
		} else {
#if NET_USE_CHANNEL
			sprintf(buffer, "AT+CIPCLOSE=%d\r\n", conn->id);
#else
			sprintf(buffer, "AT+CIPCLOSE\r\n");
#endif
		}
		simcom_sendstring(conn->netctx, (uint8 *)buffer);
		if((conn->base.mode & NET_LISTEN) == 0) {
			conn->netctx->state &= ~IF_NET_STATE_OPENED;		//switch state
		}
		conn->state = NET_CONN_CLOSED;
		net_conn_close(conn);
		net_buffer_reset();
	}
	OS_DEBUG_EXIT();
}


uint16 simcom_sock_send(net_conn_p conn, uint8 * payload, uint16 length) {
	OS_DEBUG_ENTRY(simcom_sock_send);
	uint8 lbuf[500];
	uint16 len;
	uint16 i;
	//start send request
#if ESP8266_TRANSPARENT_TRANSMISSION
	simcom_sendbytes(ctx, payload, length);
#else
#if NET_USE_CHANNEL
	sprintf((char *)lbuf, "AT+CIPSEND=%d,%d", conn->id, length);
#else
	sprintf((char *)lbuf, "AT+CIPSEND=%d", length);
#endif
	simcom_command(conn->netctx, (uint8 *)lbuf, gba_net_bufstream, 200);
	simcom_data(conn->netctx, payload, length, NULL, 0);
#endif
	OS_DEBUG_EXIT();
	return length;
}

uint16 simcom_sock_recv(net_conn_p conn, uint8 * response, uint16 bufsz) {
	OS_DEBUG_ENTRY(simcom_sock_recv);
	uint16 plen;
	uint16 len;
	uint16 i = 0;
	len = 0;
	uint16 data_left;
	uint16 timeout = 5000;
#if ESP8266_TRANSPARENT_TRANSMISSION
	len = simcom_command(ctx, NULL, response, 8000); 
#else
	//goto try_next;
	//while(1) {
	//plen = simcom_data(conn->netctx, NULL, 0, response, timeout); 
	len = (bufsz < conn->base.buflen)?bufsz:conn->base.buflen;
	//data_left = conn->base.buflen - len;
	memcpy(response, conn->base.buffer, len);
	//if(data_left == 0) {
	conn->base.buflen = 0;
	conn->base.bufend = 0;
	//	conn->state &= ~NET_CONN_BUFRDY;		//clear ready flag
	//}
	//len = simcom_compacting_net_buffer(ctx, response, plen);
#endif
	exit_tcp_recv:
	OS_DEBUG_EXIT();
	return len;
}


static void simcom_tick(net_context_p ctx) {
	uint8 ssidname[256];
	if(NET_BUSY(ctx)) return;
	//if(simcom_is_connected(ctx, ssidname) == 0) if_timer_set_tick(28);
	//else if_timer_set_tick(7);
}

uint8 simcom_power_up(net_context_p ctx) {
	uint8 cntr = 25;
	if(ctx->state & IF_NET_STATE_INITIALIZED) return 0;		//already powered up 
#if SHARD_RTOS_ENABLED 
	while(NET_BUSY(ctx)) { if_delay(100); }
#endif
	//turning on device
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3,GPIO_PIN_SET);
	if_delay(40);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	if_delay(1000);
	while((ctx->state & (IF_NET_STATE_DISABLED | IF_NET_STATE_ENABLED | IF_NET_STATE_READY)) == 0 && cntr > 0) {
		if_delay(200);
		cntr--;
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
	return 0;
}

uint8 simcom_power_down(net_context_p ctx) {
	uint8 ret = 1;
#if SHARD_RTOS_ENABLED 
	while(NET_BUSY(ctx)) { if_delay(100); }
#else 
	if(NET_BUSY(ctx)) return;
#endif
	if(g_gsm_server_channel == NULL) {
		ret = 0;
		//simcom_command(ctx, (uint8 *)"AT+GSLP=1000", NULL, 200); 				//deepsleep
	} else {
		//simcom_command(ctx, (uint8 *)"AT+SLEEP=1", NULL, 200);
	}
	return ret;
}

void simcom_deep_sleep(net_context_p ctx) {
	uint8 netbuf[256];
	//sprintf((char *)netbuf, "AT+GLSP=%d", 2000);
	//simcom_command(ctx, netbuf, netbuf, 200); 				//timeout 5 second
}

static uint8 simcom_force_connect(net_context_p ctx) {
	static uint8 try_idx = 0;
	uint8 w_idx = 0;
	uint16 len;
	uint16 i = 0,j;
	uint8 num_ssid = 0;
	uint8 cbuf[8];
	uint8 state = 0;
	uint8 mark[] = "+CWLAP:";
	net_ssid_p clist = NULL;		//current list
	net_ssid_p plist = NULL;		//previous list
	net_ssid_p tlist;
	uint8 count;
	net_ssidrec rec;
	uint8 ret = -1;
	ctx->fcnctr++;			
	if(ctx->fcnctr > 3) goto exit_try_connect;						//3 times try
	if(if_net_ssidrec_count(ctx) == 0) goto exit_try_connect;		//no matching record, abort force connect
	if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->exception_handler != NULL) 
		ctx->exception_handler(ctx, "network reconnecting");
	//try connecting to any available SSID
	//if(ctx->ssid_list != NULL) {
		//try connecting to stored ssidrec
	for(i=0;i<4;i++) {
		if(if_net_ssidrec_read(ctx, i, &rec) != 0) continue;		//invalid record
		ret = simcom_apn_join(ctx, rec.name, rec.username, rec.password);
		break;
	}
	//	try_idx = 0;		//no ssid matched with current records
	//}
	exit_try_connect:
	return ret;
}

uint8 simcom_try_connect(net_context_p ctx) {
	uint8 ret = -1;
	if(!NET_BUSY(ctx)) {
		NET_LOCK(ctx);
		if(simcom_is_connected(ctx, gba_net_bufstream) == 0)  {
			ret = 0;
		} else {
			//try connect
			if(simcom_force_connect(ctx) == 0) {
				//conected successfully
				ret = 0;
			}
		}
		exit_try_connect:
		NET_UNLOCK(ctx);
	}
	return ret;
}

uint16 simcom_sock_accept(net_conn_p conn) {
	OS_DEBUG_ENTRY(simcom_sock_accept);
	uint32 i;
	uint16 ret = -1;
	uint32 timeout = conn->timeout;
	uint32 tickstart = HAL_GetTick();
	uint32 curtick;
	conn->state |= NET_CONN_READY;
	while(((curtick = HAL_GetTick()) - tickstart) < timeout) { 
		if( (conn->state & NET_CONN_BUFRDY) != 0) {
			wait_next_data:
			ret = 0;
			conn->state &= ~NET_CONN_BUFRDY;		//clear ready flag
			os_wait(250);
			while(conn->state & NET_CONN_BUSY) { 
				os_wait(40); 
			}
			if((conn->state & NET_CONN_BUFRDY) != 0) goto wait_next_data;
			goto exit_tcp_accept;
		}
		os_wait(40);
	}
	exit_tcp_accept:
	OS_DEBUG_EXIT();
	return ret;
}

//transmit tcp request and wait for response
uint16 simcom_tcp_sock_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	OS_DEBUG_ENTRY(simcom_tcp_sock_transmit);
	uint16 len = 0;
	net_conn_p conn;
	//try to connect in-case wakeup from sleep
	if((ctx->state & IF_NET_STATE_CONNECTED) == 0) {		//not connected
		simcom_try_connect(ctx);
	}
	if((ctx->state & IF_NET_STATE_CONNECTED) != 0) {			//not connected
		NET_LOCK(ctx);
		if((conn = simcom_sock_open(ctx, host, port, NET_TRANSMIT |NET_TYPE_TCP)) == NULL)  {
			//error connecting to server
			//NET_UNLOCK(ctx);
			goto exit_tcp_transmit;
		}
		//set receive buffer to global net buffer
		simcom_set_buffer(conn, gba_net_buffer, 16384);
		//start transmit
		simcom_sock_send(conn, payload, length);
		len = 0;
		if(simcom_sock_accept(conn) == 0) {
			len = simcom_sock_recv(conn, response, conn->base.bufsz);
		}
		exit_tcp_transmit:
		//close connection
		simcom_sock_close(conn);
		NET_UNLOCK(ctx);
	}
	OS_DEBUG_EXIT();
	return len;		//unable to found +IPD prefix
}

uint16 simcom_udp_sock_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	OS_DEBUG_ENTRY(simcom_udp_sock_transmit);
	uint16 len = 0;
	net_conn_p conn;
	//try to connect in-case wakeup from sleep
	if((ctx->state & IF_NET_STATE_CONNECTED) == 0) {		//not connected
		simcom_try_connect(ctx);
	}
	if((ctx->state & IF_NET_STATE_CONNECTED) != 0) {			//not connected
		NET_LOCK(ctx);
		if((conn = simcom_sock_open(ctx, host, port, NET_TRANSMIT |NET_TYPE_UDP)) == NULL)  {
			//error connecting to server
			//NET_UNLOCK(ctx);
			goto exit_tcp_transmit;
		}
		//set receive buffer to global net buffer
		simcom_set_buffer(conn, gba_net_buffer, 16384);
		//start transmit
		simcom_sock_send(conn, payload, length);
		len = 0;
		if(simcom_sock_accept(conn) == 0) {
			len = simcom_sock_recv(conn, response, conn->base.bufsz);
		}
		exit_tcp_transmit:
		//close connection
		simcom_sock_close(conn);
		NET_UNLOCK(ctx);
	}
	OS_DEBUG_EXIT();
	return len;		//unable to found +IPD prefix
}

uint8 simcom_enable(net_context_p ctx) {
	return simcom_try_connect(ctx);
}

uint8 simcom_disable(net_context_p ctx) {
	uint8 netbuf[256];
	uint8 len;
	len = simcom_command(ctx, (uint8 *)"AT+CIPSHUT", netbuf, 400);
	netbuf[len] = 0;
	if(strstr((const char *)netbuf, "OK") != 0) return 0;
	return -1;
}

//uint8 g_gsm_task_stack[1024];
uint8 simcom_init(net_context_p ctx) {
	//net_ssid_p list;
	uint16 len;  
	uint8 i, n1, n2, n3;
	char * ptr;
	uint8 retry_cnt = 3;
	uint8 pdp_state = 0;
	uint8 netbuf[2048];
	if(os_find_task_by_name("gsm") == NULL) {
		os_create_task_static(ctx, if_simcom_task, "gsm", 60, (lp_void)NULL, 4096);
	}
	restart_simcom_init:
	retry_cnt --; 
	if(retry_cnt == 0) return -1;
	if((ctx->state & IF_NET_STATE_INITIALIZED) == 0) {
		sim8xx_init_config(ctx);			//initialize pin and usart1 (ready to transmit data)
		len = simcom_command(ctx, (uint8 *)"AT+CGATT?", netbuf, 2000);
		if(len == 0) {
			simcom_power_up(ctx);
			//os_wait(6000);
			// 
			//	len = simcom_command(ctx, (uint8 *)"AT+CGATT?", netbuf, 200);
			//len = simcom_data(ctx, NULL, 0, netbuf, 10000);
			if((ctx->state & (IF_NET_STATE_READY | IF_NET_STATE_ENABLED)) == 0) 
				goto reshut_module;
			len = simcom_command(ctx, (uint8 *)"AT+CGATT?", netbuf, 2000);
		} else {
			//device already powered up
			reshut_module:
			//set module to IP INITIAL
			len = simcom_command(ctx, (uint8 *)"AT+CIPSHUT", netbuf, 400);
			netbuf[len] = 0;
			if(len  == 0) goto restart_simcom_init;
			if(strstr((const char *)netbuf, "SHUT OK") != NULL) pdp_state = 1;
			else  pdp_state = 0;
			while(1) {
				len = simcom_data(ctx, NULL, 0, netbuf, 5000);
				netbuf[len] = 0;
				if(len == 0) {
					if(pdp_state) {
						len = simcom_command(ctx, (uint8 *)"AT+CGATT?", netbuf, 2000);
						break;
					} 
					goto reshut_module;
				}
				if(strstr((const char *)netbuf, "NORMAL POWER DOWN") != 0) {
					os_wait(5000);
					goto reshut_module; 
					//len = simcom_command(ctx, (uint8 *)"AT+CGATT?", netbuf, 2000);
				}
				if((ctx->state & (IF_NET_STATE_READY | IF_NET_STATE_ENABLED)) == 0)
					goto restart_simcom_init;
				else 
					break;
			}
		}
	}
	if(len == 0) { 
		goto restart_simcom_init; 
	}
	//os_wait(1000);
	len = simcom_command(ctx, (uint8_t *)"AT+CFUN=1", netbuf, 500); 					//full functionality
	netbuf[len] = 0;
	if(strstr((const char *)netbuf, "OK") == NULL) { retry_cnt --; goto restart_simcom_init; }
	simcom_command(ctx, (uint8_t *)"AT+CIPMODE=0", netbuf, 500); 					//normal mode
	simcom_command(ctx, (uint8_t *)"AT+CIPSRIP=1", netbuf, 500); 					//show remote ip and port
	
#if NET_USE_CHANNEL
	simcom_command(ctx, (uint8_t *)"AT+CIPMUX=1", netbuf, 500); 					//single channel
#endif
	simcom_command(ctx, (uint8_t *)"AT+GSMBUSY=1", netbuf, 500); 	//reject all incoming call
	
	if(len != 0 && (ctx->state & IF_NET_STATE_INITIALIZED) == 0) {
		ctx->state |= IF_NET_STATE_INITIALIZED | IF_NET_STATE_GPS_AVAIL;
		memset(ctx->ipv4, 0, 4);
		
		//device APIs
		ctx->type = NET_TYPE_GSM;
		ctx->init = simcom_init;
		ctx->join = simcom_apn_join;
		ctx->list = simcom_ssid_list;
		ctx->try_connect = simcom_try_connect;
		ctx->sleep = simcom_sleep;
		ctx->get_status = simcom_get_status;
		ctx->get_config = simcom_get_ipconfig;
		ctx->power_down = simcom_power_down;
		ctx->power_up = simcom_power_up;
		ctx->enable = simcom_enable;
		ctx->disable = simcom_disable;
		
		//socket APIs
		ctx->sock_open = simcom_sock_open;
		ctx->sock_send = simcom_sock_send;
		ctx->sock_recv = simcom_sock_recv;
		ctx->sock_close = simcom_sock_close;
		
		//transport APIs
		ctx->send_tcp = simcom_tcp_sock_transmit;
		ctx->send_udp = simcom_udp_sock_transmit;
		ctx->exception_handler = NULL;
		ctx->p_ctx = NULL;
		ctx->f_ctx = NULL;
		ctx->prepare_callback = NULL;
		ctx->finish_callback = NULL;
		ctx->cb_depth = 0;
		ctx->fcnctr = 0;
		os_wait(200);
	} else 
		return -1;
	return 0;
}

static uint16 simcom_ble_command(bt_context_p ctx, uint8_t * command, uint8 * response, uint32 timeout) {
	uint16 len = 0;
	len = simcom_command(ctx->handle, command, response, 40);
	len += simcom_data(ctx->handle, NULL, 0, response + len, timeout);
	return len;
}

uint16 sim_hex2bin(uint8 * hexstring, uint8 * bytes, uint8 length) {
	uint16 i = 0;
	uint8 c;
	uint16 len=0;
	while(i < length) {
		if(i & 0x01) {
			c <<= 4;
			c |= net_hex2byte(hexstring[i]);
			bytes[len] = c;
			len++;
		} else {
			c = net_hex2byte(hexstring[i]);
		}
		i++;
	}
	return len;
}

void sim_mac2bin(uint8* mac_str, uint8 * bin) {
	uint8 c;
	uint8 i;
	for(i=0;i<6;i++) {
		c = net_hex2byte(mac_str[i * 3]);
		c <<= 4;
		c |= net_hex2byte(mac_str[(i * 3) + 1]);
		bin[i] = c;
	}
}

static bt_device_p simcom_ble_device_create(bt_context_p ctx, uint8 * mac) {
	uint8 i = 0;
	uint8 dev_macs[256];
	uint8 state = 0;
	uint8 parsing = 1;
	uint8 bufidx = 0;
	bt_device_p dev = (bt_device_p)malloc(sizeof(bt_device));
	memset(dev_macs, 0, sizeof(dev_macs));
	if(dev == NULL) return NULL;
	memcpy(dev_macs, mac, 17);
	sim_mac2bin(dev_macs, dev->mac);
	sprintf((char *)dev->name, "%02x:%02x:%02x:%02x:%02x:%02x", dev->mac[0], dev->mac[1], dev->mac[2], dev->mac[3], dev->mac[4], dev->mac[5]);
	dev->next = NULL;
	dev->ctx = ctx;
	dev->srv_id = 0;
	dev->chr_id = 0;
	return dev;
}

void simcom_ble_device_clear(bt_device_p root) {
	bt_device_p tlist = root;
	bt_device_p plist;
	while(tlist != NULL) {
		plist = tlist;
		tlist = tlist->next;
		memset(plist, 0, sizeof(bt_device));
		free(plist);
	}
}

uint8 simcom_ble_get_mac(bt_context_p ctx) {
	uint8 buffer[256];
	uint8 len;
	char * ptr;
	uint8 ret = -1;
	memset(buffer, 0, sizeof(buffer));
	len = simcom_ble_command(ctx, (uint8 *)"AT+BLEADDR?", buffer, 200);
	if((ptr = strstr((char *)buffer, "+BLEADDR:")) != NULL) {
		sim_mac2bin((uint8 *)ptr + 9, ctx->host_mac);
		ret = 0;
	}
	return ret;
}

uint8 simcom_ble_dev_list(bt_context * ctx) {
	OS_DEBUG_ENTRY(simcom_ble_dev_list);
	uint8 buffer[2400];
	uint8 mac[8];
	bt_device_p iterator = NULL;
	uint8 dev_count = 0;
	uint8 len;
	uint8 * ptr;
	if(ctx->state & BLE_STATE_CONNECTED) {
		iterator = ctx->dev_list;
		while(iterator != NULL) {
			dev_count++;
			iterator = iterator->next;
		}
		goto exit_dev_list;
	}
	len = simcom_ble_command(ctx, (uint8 *)"AT+BLESCAN=1,1", buffer, 400);		//scan for 2 seconds
	if(len != 0) { simcom_ble_device_clear(ctx->dev_list); ctx->dev_list = NULL; }
	iterator = ctx->dev_list;
	ptr = buffer;
	buffer[len] = 0;
	while((ptr = (uint8 *)strstr((char *)ptr, "+BLESCAN:")) != NULL) {
		if(iterator == NULL) {
			iterator = simcom_ble_device_create(ctx, ptr + 9);		//mac address
			ctx->dev_list = iterator;
			dev_count ++;
		} else {
			sim_mac2bin(ptr + 9, mac);
			if(if_ble_dev_check(ctx, mac) == NULL) {			//check if device already in the list
				iterator->next = simcom_ble_device_create(ctx, ptr + 9);		//mac address
				dev_count ++;
			}
		}
		ptr++;		//advanced pointer to next character
	}
	exit_dev_list:
	OS_DEBUG_EXIT();
	return dev_count;
}

uint8 simcom_ble_is_connected(bt_context_p ctx, uint8 * mac) {
	OS_DEBUG_ENTRY(simcom_ble_is_connected);
	uint8 buffer[2048];
	uint8 len;
	uint8 ret = -1;
	uint8 hex_buffer[32];
	char * ptr;
	if(ctx->state & BLE_STATE_INITIALIZED) {
		len = simcom_ble_command(ctx, (uint8 *)"AT+BLEINIT=1", buffer, 200);
		buffer[len] = 0;
		len = simcom_ble_command(ctx, (uint8 *)"AT+BLECONN?", buffer, 400);
		buffer[len] = 0;
		if((ptr = strstr((char *)buffer, "+BLECONN:0")) != NULL) {
			ctx->state |= BLE_STATE_CONNECTED;
			if(mac != NULL) {
				sim_mac2bin((uint8 *)ptr + 11, mac);
			}
			ret = 0;
		} else {
			ctx->state &= ~BLE_STATE_CONNECTED;
		}
	}
	exit_is_connect:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 simcom_ble_wake(bt_context * ctx) {
	uint16 len;
	uint8 ret = -1;
	uint8 buffer[64];
	if((ctx->state & BLE_STATE_INITIALIZED) == 0) return ret;
	len = simcom_ble_command(ctx, (uint8 *)"AT+BLEINIT=1", buffer, 200);
	if(strstr((char *)buffer, "OK") != NULL) {
		ctx->state |= BLE_STATE_INITIALIZED;
		ret = 0;
	}
	return ret;
}

bt_service_p simcom_ble_list_services(bt_context_p ctx) {
	OS_DEBUG_ENTRY(simcom_ble_list_services);
	uint16 len;
	uint8 * ptr;
	#define SBUFFER_SIZE		1596
	uint8 sbuffer[SBUFFER_SIZE];
	uint16 sindex = 0;
	bt_service * prev = NULL ;
	bt_service * iterator = NULL;
	bt_chr * chr = NULL;
	bt_chr * prev_chr = NULL;
	uint8 buffer[500];
	uint32 diff;
	uint8 srv_count = 0;
	if(ctx == NULL) goto exit_ble_list_services;
	if((ctx->state & BLE_STATE_CONNECTED) == 0) goto exit_ble_list_services;			//no device connected
	//list all available services
	len = simcom_ble_command(ctx, (uint8 *)"AT+BLEGATTCPRIMSRV=0", gba_net_bufstream, 400);
	ptr = gba_net_bufstream;
	gba_net_bufstream[len] = 0;
	while((ptr = (uint8 *)strstr((char *)ptr, "+BLEGATTCPRIMSRV:")) != NULL) {
		//ptr +19
		iterator = (bt_service *)((uint8 *)sbuffer + sindex);
		iterator->srv_id = net_hex2byte(ptr[19]);
		iterator->chrs = NULL;
		iterator->next = NULL;
		iterator->rsv = 0;
		iterator->state = 0;
		if(ptr[27] == ',') {
			sim_hex2bin(ptr + 23, iterator->srv_uuid, 4);
			iterator->srv_uuid_len = 2;
			sindex += (sizeof(bt_service) + 2);
		} else {
			sim_hex2bin(ptr + 23, iterator->srv_uuid, 32);
			iterator->srv_uuid_len = 16;
			sindex += (sizeof(bt_service) + 16);
		}
		if(prev != NULL) {
			prev->next = iterator;
		}
		prev = iterator;
		srv_count++;
		//check if sbuffer is almost full
		if(sindex > (SBUFFER_SIZE - 32)) break;
		ptr ++;
	}
	if(srv_count == 0) goto exit_ble_list_services;			//return NULL (no service available)
	iterator = (bt_service *)sbuffer;
	//iterate each services for characteristics
	while(iterator != NULL) {
		//list characteristics of each services
		sprintf((char *)buffer, "AT+BLEGATTCCHAR=0,%d", iterator->srv_id);
		len = simcom_ble_command(ctx, (uint8 *)buffer, gba_net_bufstream, 200);
		ptr = gba_net_bufstream;
		gba_net_bufstream[len] = 0;
		prev_chr = NULL;	
		chr = NULL;
		while((ptr = (uint8 *)strstr((char *)ptr, "+BLEGATTCCHAR:")) != NULL) {
			chr = (bt_chr *)((uint8 *)sbuffer + sindex);
			if(iterator->chrs == NULL) {			//set service characteristics
				iterator->chrs = chr;
			}
			chr->chr_id = net_hex2byte(ptr[25]);
			chr->service = iterator;
			chr->next = NULL;
			chr->state = 0;
			//memcpy(chr->chr_uuid, ptr+29
			if(ptr[15] == 'c') {
				sim_hex2bin(ptr + 29, chr->chr_uuid , 4);
				chr->attr = (net_hex2byte(ptr[36]) << 4) | net_hex2byte(ptr[37]);		//set attribute for current characteristic
				chr->chr_uuid_len = 2;
			} else {
				sim_hex2bin(ptr + 31, chr->chr_uuid , 4);
				chr->chr_uuid_len = 2;
				chr->attr = 0;
			}
			sindex += (sizeof(bt_chr) + 2);
			if(prev_chr != NULL) {
				prev_chr->next = chr;
			}
			prev_chr = chr;
			//check if sbuffer is almost full
			if(sindex > (SBUFFER_SIZE - 32)) break;
			ptr ++;
		}
		if(sindex > (SBUFFER_SIZE - 32)) break;
		iterator = iterator->next;
	}
	//relocate buffer to heap
	prev = iterator = os_alloc(sindex);
	if(iterator != NULL) {
		memcpy(iterator, sbuffer, sindex);
		//iterate each services for pointer relocation
		diff = (uint32)iterator - (uint32)sbuffer;
		while(iterator != NULL) {
			if(iterator->next != NULL) iterator->next = (bt_service *)((uint8 *)(iterator->next) + diff);
			if(iterator->chrs != NULL) iterator->chrs = (bt_chr *)((uint8 *)(iterator->chrs) + diff);
			//iterate each characteristics for pointer relocation
			chr = iterator->chrs;
			while(chr != NULL) {
				if(chr->next != NULL) chr->next = (bt_chr *)((uint8 *)(chr->next) + diff);
				if(chr->service != NULL) chr->service = (bt_service *)((uint8 *)(chr->service) + diff);
				chr = chr->next;
			}
			iterator = iterator->next;
		}
	}
	iterator = prev;
	exit_ble_list_services:
	OS_DEBUG_EXIT();
	return iterator;
}

uint8 simcom_ble_disconnect(bt_context * ctx) {
	uint8 buffer[2048];
	uint8 len = 0;
	uint8 ret = -1;
	if((ctx->state & BLE_STATE_CONNECTED) == 0) return ret;
	len = simcom_ble_command(ctx, (uint8 *)"AT+BLEDISCONN=0", buffer, 200);
	if(strstr((char *)buffer, "OK") != NULL) { 
		ctx->state &= ~(BLE_STATE_CONNECTED | BLE_STATE_OPENED);		//close and disconnect
		if(ctx->services != NULL) os_free(ctx->services);
		ctx->services = NULL;
		ret = 0;
	}
	return ret;
}

uint8 simcom_ble_connect(bt_context * ctx, bt_device_p dev) {
	OS_DEBUG_ENTRY(simcom_ble_connect);
	uint8 buffer[2048];
	uint8 len;
	uint8 ret = -1;
	uint8 hex_buffer[32];
	char * ptr;
	sprintf((char *)hex_buffer, "%02x:%02x:%02x:%02x:%02x:%02x", dev->mac[0], dev->mac[1], dev->mac[2], dev->mac[3], dev->mac[4], dev->mac[5]);
	sprintf((char *)buffer, "AT+BLECONN=0,\"%s\"", hex_buffer);		//use connection index=0
	if(ctx->state & BLE_STATE_CONNECTED) simcom_ble_disconnect(ctx);
	//if(ctx->state & IF_BLE_CONNECTED) goto exit_connect;	//already connected
	if(ctx->state & BLE_STATE_INITIALIZED) {
		if((ctx->handle->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->handle->exception_handler != NULL) 
			ctx->handle->exception_handler(ctx->handle, "bluetooth connecting");
		len = simcom_ble_command(ctx, buffer, buffer, 400);
		if((ptr = strstr((char *)buffer, "+BLECONN:")) != NULL) {
			memcpy(ctx->slave_mac, dev->mac, 6);
			if((ctx->handle->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->handle->exception_handler != NULL) 
				ctx->handle->exception_handler(ctx->handle, "enumerating services");
			ctx->state |= BLE_STATE_CONNECTED;
			ctx->services = simcom_ble_list_services(ctx);
			if_ble_btrec_push(NULL, dev->mac, NULL);
			ret = 0;
		}
	}
	exit_connect:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 simcom_ble_write(bt_handle_p handle, uint8 * data, uint16 size) {
	OS_DEBUG_ENTRY(simcom_ble_write);
	uint16 len;
	uint8 ret = -1;
	uint8 buffer[1500];
	uint8 chid = handle->chr->chr_id;
	uint8 svid = handle->chr->service->srv_id;
	if(handle == NULL) goto exit_ble_write;
	sprintf((char *)buffer, "AT+BLEGATTCWR=0,%d,%d,,%d", svid, chid, size);
	len = simcom_command(handle->ctx->handle, buffer, buffer, 200);
	buffer[len] = 0;
	if(strstr((const char *)buffer, ">") != NULL) {
		simcom_data(handle->ctx->handle, data, size, NULL, 500);
		ret = 0;
	}
	exit_ble_write:
	OS_DEBUG_EXIT();
	return ret;
}

uint16 simcom_ble_read(bt_handle_p handle, uint8 * response, uint16 size) {
	OS_DEBUG_ENTRY(simcom_ble_read);
	uint16 len;
	uint8 i;
	uint16 ret = 0;
	uint8 buffer[1500];
	uint8 * ptr;
	uint8 chid = handle->chr->chr_id;
	uint8 svid = handle->chr->service->srv_id;
	if(handle == NULL) goto exit_ble_read;
	//activate service
	sprintf((char *)buffer, "AT+BLEGATTCCHAR=0,%d", svid);
	len = simcom_ble_command(handle->ctx, buffer, buffer, 200);
	//start read characteristics
	sprintf((char *)buffer, "AT+BLEGATTCRD=0,%d,%d", svid, chid);
	//len = simcom_command(handle->ctx->handle, buffer, buffer, 200);
	len = simcom_ble_command(handle->ctx, buffer, response, 400);
	if((ptr = (uint8 *)strstr((const char *)response, "+BLEGATTCRD:")) != NULL) {
		for(i=14;i<18;i++) { if(ptr[i] == ',') { response[i] = 0; break; } }
		ret = atoi((const char *)ptr + 14);
		memcpy(response, ptr + (i + 1), ret);
	}
	exit_ble_read:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 simcom_ble_init(bt_context * ctx, void * handle) {
	return -1;
}

void simcom_gps_power_up(gps_context * ctx) {
	uint8 gpsbuffer[128];
	simcom_command(ctx->handle, (uint8 *)"AT+CGNSPWR=1", gpsbuffer, 200);	//power up device
	//simcom_command(ctx->handle, (uint8 *)"AT+CGNSTST=1", gpsbuffer, 200);	//re-route uart2 to uart1
	simcom_command(ctx->handle, (uint8 *)"AT+CGNS=1,1", gpsbuffer, 200);	//power up device
	simcom_command(ctx->handle, (uint8 *)"AT+CGNSSEQ=\"RMC\"", gpsbuffer, 200);
	simcom_command(ctx->handle, (uint8 *)"AT+CGNSVER", gpsbuffer, 200);
	//hot-start GNSS
	//simcom_command(ctx->handle, (uint8 *)"AT+CGNSCMD=0,\"$PMTK102*31\"", gpsbuffer, 200);
	//easy mode
	//simcom_command(ctx->handle, (uint8 *)"AT+CGNSCMD=0,\"$PMTK869,1,1*35\"", gpsbuffer, 200);
}

void simcom_gps_power_down(gps_context * ctx) {
	uint8 gpsbuffer[128];
	simcom_command(ctx->handle, (uint8 *)"AT+CGNSPWR=0", gpsbuffer, 200);	//power down device
}

void simcom_gps_reset(gps_context * ctx) {
}

uint8 simcom_gps_get(gps_context * ctx) {
	uint8 ret = -1;
	uint8 gpsbuffer[512];
	uint8 temp[32];
	uint8 tlen = 0;
	uint8 state = 0;
	uint16 len;
	uint8 c;
	uint8 * ptr = gpsbuffer;
	while(GPS_BUSY(ctx)) { if_delay(100); }
	GPS_LOCK(ctx);
	len = simcom_command(ctx->handle, (uint8 *)"AT+CNTP", gpsbuffer, 400);
	len = simcom_command(ctx->handle, (uint8 *)"AT+CGNSINF", gpsbuffer, 200);	//power down device
	gpsbuffer[len] = 0;
	/*
	+CGNSINF: 	<GNSS run status>,		0/1
				<Fix status>, 			0/1
				<UTC date & Time>,		yyyyMMddhhmmss.sss -> 20161215045641.000
				<Latitude>,				(+/-)dd.dddddd -> 31.221303
				<Longitude>, 			(+/-)ddd.dddddd -> 31.221303
				<MSL Altitude>,			float string[8] -> 71.900 (meters)
				<Speed Over Ground>, 	[0,999.99] -> km/h
				<Course Over Ground>, 	[0,360.00] -> degree
				<Fix Mode>,				0/1/2
				<Reserved1>,
				<HDOP>,					[0,99.9]
				<PDOP>, 				[0,99.9]
				<VDOP>,					[0,99.9]
				<Reserved2>,
				<GNSS Satellites in View>, 	[0,99] 
				<GNSS Satellites Used>,		[0,99] 
				<GLONASS Satellites Used>,	[0,99] 
				<Reserved3>,
				<C/N0 max>,				[0-55]	(dBHz)
				<HPA>,					float string[6] -> (meters)
				<VPA>  					float string[6] -> (meters)
				examples : 1,1,20161215045641.000,31.221303,121.355042
				,71.900,0.00,45.1,1,,1.0,1.3,0.8,,10,10,,,36,,
	*/
	if((ptr = (uint8 *)strstr((const char *)ptr, "+CGNSINF:")) != NULL) {
		ptr += 9;
		for(state=0;state<21;state++) {
			while((c = *ptr++) != 0) {
				if(c == ',') break;
				if(c == '\r') break;
				if(c == '\n') break;
				temp[tlen++] = c;
			}
			if(c == 0) break;
			if(c == '\r') break;
			if(c == '\n') break;
			temp[tlen++] = 0;
			tlen = 0;
			switch(state) {
				case 0:		//<GNSS run status>
					if(atoi((const char *)temp) != 1) {
						ctx->state &= ~IF_GPS_STATE_READY;
						goto exit_gps_get;
					}
					break;
				case 1:		//<Fix status>
					if(atoi((const char *)temp) == 1) {
						ret = 0;
					}
					break;
				case 2:		//<UTC date & Time>
					ctx->stamp = tk_iso8601_decode((char *)temp);
					break;
				case 3:		//<Latitude>
					ctx->latitude = atof((const char *)temp);
					break;
				case 4:		//<Longitude>
					ctx->longitude = atof((const char *)temp);
					break;
				case 5:		//<MSL Altitude>
					ctx->altitude = atoi((const char *)temp);
					break;
				case 6:		//<Speed Over Ground>
					ctx->velocity = atoi((const char *)temp);
					break;
				default: break;
			}
		}
	}
	exit_gps_get:
	GPS_UNLOCK(ctx);
	return ret;
}

uint8 simcom_gps_init(gps_context * ctx, void * handle) {
	net_context_p netctx = handle;
	uint8 gpsbuffer[128];
	uint16 len;
	if(netctx == NULL) return -1;
	if((netctx->state & IF_NET_STATE_GPS_AVAIL) == 0) return -1;
	if(netctx->init == NULL) return -1;
	ctx->handle = handle;
	ctx->state = IF_GPS_STATE_INITIALIZED | IF_GPS_STATE_READY;
	ctx->init = simcom_gps_init;
	ctx->power_up = simcom_gps_power_up;
	ctx->power_down = simcom_gps_power_down;
	ctx->reset = simcom_gps_reset;
	ctx->get = simcom_gps_get;
	return 0;
}