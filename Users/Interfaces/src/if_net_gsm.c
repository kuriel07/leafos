#include "defs.h"
#include "config.h"
#include <string.h>
#include "..\inc\if_apis.h"
#include "..\inc\if_net.h"
#include "if_net.h"
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif

#if SHARD_WIFI_MODULE == MOD_ESP8266 || SHARD_WIFI_MODULE == MOD_ESP32
#define ESP8266_TRANSPARENT_TRANSMISSION		0
//ESP8266 driver (ESP12-E)
//created 2016.03.08
//direct access to CMSIS
//successfully connected to AP (2016.03.12, roshi's birthday)
//use GPIOA.9 TX, GPIOA.10 RX, GPIOA.8 RST, GPIOA.11 ChipEn
//fixed: sending http request (closing command detect) when receving data before connection closed (2016.03.31)
//uint8 gba_net_buffer[1024];
//bug: if request cannot be made either because the timeout on tcp_accept is too little
//added ssl transmit for if_net_task (2017.04.15)
//fixed UDP send, because it's fast, do no wait for response (2017.06.20)
//added debug entry (2017.06.22)
//fixed if_net_tcp_recv and if_net_udp_recv when receiving several bytes of chunk, replace a byte with zero (2017.07.28)
//added support for ESP32 and BLE module (2018.03.22)
//added if_net_decode, if_net_exec_listener (2018.04.22)

#define NET_STAT_BUF_RDY			0x01
#define NET_STAT_BUF_FULL			0x02
#define NET_STAT_CLEAR				0x00
uint8 gba_net_buffer[NET_BUFFER_SIZE];
#define gba_net_bufstream (gba_net_buffer + (NET_BUFFER_SIZE >> 1))
uint16 gba_net_head = 0;
uint16 gba_net_tail = 0;
uint8 gb_net_stat = 0;
static net_context_p g_current_context = NULL;
net_conn * g_channels[NET_MAX_CHANNEL] = { NULL, NULL, NULL, NULL };		//global channels
net_conn * g_server_channel = NULL;

void net_mac2bin(uint8* mac_str, uint8 * bin);

static uint8 net_hex2byte(uint8 hexchar) {
	if(hexchar >= 'a' && hexchar <= 'f') return (hexchar - 'a') + 10;
	if(hexchar >= 'A' && hexchar <= 'F') return (hexchar - 'A') + 10;
	if(hexchar >= '0' && hexchar <= '9') return hexchar - '0';
	return 0;
}

void net_buffer_reset() {
	if(gba_net_head == gba_net_tail) {
		gba_net_head = 0;
		gba_net_tail = 0;
	}
}

void net_buffer_push(uint8 c) {
	gba_net_bufstream[gba_net_head++] = c;
}

uint8 net_buffer_dequeue(uint8 * c) {
	if(gba_net_tail >= gba_net_head) return -1;
	c[0] = gba_net_bufstream[gba_net_tail++];
	return 0;
}

uint16 net_buffer_length() {
	return gba_net_head;
}

uint16 net_buffer_tail() {
	return gba_net_tail;
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
#define ND_CMD_WIFI_CONNECTED 		27
#define ND_CMD_WIFI_GOT_IP			28
#define ND_CMD_WIFI_DISCONNECTED	29
#define ND_CMD_SEND_OK			31

typedef struct net_def_state net_def_state;
struct net_def_state {
	uint8 length;
	const char * keyword;
	uint8 state;
};

const net_def_state g_states[] = {
	{ 5, "+IPD,", ND_WAIT_DATA_CHANNEL },
	{ 7, "SEND OK", ND_CMD_END },
	{ 2, "OK", ND_CMD_END },
	{ 5, "ERROR", ND_CMD_END },
	{ 7, "CONNECT", ND_CMD_CONNECTED },
	{ 6, "CLOSED", ND_CMD_CLOSED },
	{ 14, "WIFI CONNECTED", ND_CMD_WIFI_CONNECTED },
	{ 11, "WIFI GOT IP", ND_CMD_WIFI_GOT_IP },
	{ 17, "WIFI DISCONNECTED", ND_CMD_WIFI_DISCONNECTED },
	{ 0, NULL, ND_WAIT_STATUS },
};

void if_net_set_listen_callback(net_conn_p conn, void * ctx, void (* callback)(void *, net_buffer_p)) {
	if(conn == NULL) return;
	conn->base.ctx = ctx;
	conn->base.listen_callback = callback;
}

void if_net_set_timeout(net_conn_p conn, uint16 timeout) {
	conn->timeout = timeout;
}

uint8 if_net_exec_listener(net_context_p ctx) {
	net_conn_p conn = NULL;
	uint8 i=0;
	uint8 ret = -1;
	uint16 buflen = 0;
	for(i=0;i<NET_MAX_CHANNEL;i++) {
		conn = g_channels[i];
		if(conn != NULL) {
			if((conn->state & NET_CONN_BUFRDY) != 0) {
				//data exist, check available buffer length (do not clear flag until callback executed)
				while(buflen != conn->base.buflen) {
					buflen = conn->base.buflen;
					os_wait(250);			//wait for multiple data
				}
				if(conn->base.listen_callback != NULL) conn->base.listen_callback(conn->base.ctx, conn);	//exec callback
				conn->state &= ~NET_CONN_BUFRDY;	//clear flag
				conn->base.buflen = 0;					//clear buffer;
				conn->base.bufend = 0;
				ret = 0;
				break;
			}
		}
	}
	return ret;
}

static uint8 if_net_decode(net_context_p ctx) {
	static uint8 res_len = 0;
	static uint16 res_index = 0;
	static uint8 state = ND_WAIT_STATUS;
	static int16 cur_channel = -1;
	static uint8 temp[20];
	static uint8 remote_ip[4];
	static uint16 remote_port;
	static uint8 temp_index = 0;
	static uint16 data_length = 0;
	static uint8 lost_cntr = 100;
	uint8 temp_ip[4];
	//static net_status status[4];
	net_conn * conn;
	uint32 start_tick;
	//uint8 total, i;
	//uint16 local_port;
	uint8 c;
	uint8 index = 0;
	if(net_buffer_dequeue(&c) != 0) return -1; 	//{
	//	lost_cntr--;
	//	if(lost_cntr == 0) {
	//		lost_cntr = 100;
	//		state = ND_WAIT_STATUS;	//reset state if no data buffer for at-least 4000ms
	//	}
	//	return -1;
	//}
	//lost_cntr = 100;
	switch(state) {
		case ND_CMD_WIFI_DISCONNECTED:
			ctx->state &= ~(IF_NET_STATE_CONNECTED | IF_NET_STATE_IP_ASSIGNED);		//clear status
			goto wait_status;
		case ND_CMD_WIFI_CONNECTED:
			ctx->state |= IF_NET_STATE_CONNECTED;
			goto wait_status;
		case ND_CMD_WIFI_GOT_IP:
			ctx->state |= IF_NET_STATE_IP_ASSIGNED;
			goto wait_status;
		case ND_CMD_CLOSED:
			if(cur_channel != -1 && g_channels[cur_channel] != NULL)
				g_channels[cur_channel]->state = NET_CONN_CLOSED;
			goto wait_status;
		case ND_CMD_CONNECTED:
			if(cur_channel != -1 && g_channels[cur_channel] != NULL)
				g_channels[cur_channel]->state |= NET_CONN_CONNECTED;
			goto wait_status;
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
					if(g_states[index].length == 0) break;		//end of matching states
					if(g_states[index].length == res_len) {
						if(memcmp(g_states[index].keyword, gba_net_bufstream + res_index, res_len) == 0) {
							//matching keyword
							if(strcmp(g_states[index].keyword, "SEND OK") == 0) {
								temp_index = 0;
							}
							state = g_states[index].state;
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
				gb_net_stat |= NET_STAT_BUF_RDY;		//buffer ready
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
			conn = g_channels[cur_channel];
			if(conn == NULL) conn = g_server_channel;		//unknown channel, use server channel
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
			conn = g_channels[cur_channel];
			if(conn == NULL) conn = g_server_channel;		//unknown channel, use server channel
			//conn = g_channels[cur_channel];
			if(c == ',') {
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
			if(c == ':') {
				temp[temp_index] = 0;
				remote_port = atoi((const char *)temp);
				temp_index = 0;
				state = ND_WAIT_DATA;
			} else {
				temp[temp_index++] = c;
			}
			break;
		case ND_WAIT_DATA:
			conn = g_channels[cur_channel];
			if(conn == NULL) conn = g_server_channel;		//unknown channel, use server channel
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
					//gb_net_stat |= NET_STAT_BUF_RDY;		//buffer ready
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

net_conn * net_conn_create(net_context_p ctx, uint16 mode, uint32 timeout, uint16 port, uint8 * buffer, uint16 bufsz, void (* listen_callback)(void *, net_conn *)) {
	uint8 id = 0;
	net_conn * conn = NULL;
	//check if port is already opened
	if(mode & NET_LISTEN) {
		for(;id<NET_MAX_CHANNEL;id++) {
			if(g_channels[id] != NULL && g_channels[id]->port == port) return NULL;
		}
	}
	//try locating a link number
	id = 0;
	if(mode & NET_LISTEN) id = 1;		//listen not available on channel 0
	for(;id<NET_MAX_CHANNEL;id++) {
		if(g_channels[id] == NULL) break;
		
	}
	if(id == NET_MAX_CHANNEL) return conn;		//unable to reserve channel
	conn = (net_conn *)os_alloc(sizeof(net_conn));
	if(buffer == NULL) {
		buffer = (uint8 *)os_alloc(bufsz);
	}
	if(conn == NULL) return conn;
	memset(conn, 0, sizeof(net_conn));
	conn->base.mode = mode;
	conn->base.buffer = buffer;
	conn->base.bufsz = bufsz;		//maximum bufsz
	conn->base.buflen = 0;			//no data available
	conn->base.bufend = 0;
	conn->base.buf_release = os_free;
	conn->base.listen_callback = listen_callback;
	conn->id = id;
	conn->state = NET_CONN_READY;
	conn->netctx = ctx;
	conn->port = 0;
	if(mode & NET_LISTEN) conn->port = port;
	else conn->remote_port = port;
	conn->timeout = timeout;
	g_channels[id % NET_MAX_CHANNEL] = conn;		//assign conn to channel
	return conn;
}

void net_conn_close(net_conn * conn) {
	if(conn == NULL) return;
	g_channels[conn->id % NET_MAX_CHANNEL] = NULL;		//remove from channel list
	if(conn->base.buf_release != NULL) conn->base.buf_release(conn->base.buffer);
	os_free(conn);
}

void if_net_set_buffer(net_conn_p conn, uint8 * buffer, uint16 bufsz) {
	if(conn->base.buf_release != NULL) conn->base.buf_release(conn->base.buffer);
	conn->base.buffer = buffer;
	conn->base.bufsz = bufsz;
	conn->base.buf_release = NULL;
}

void if_wifi_task(void) {
	//task for handling response from wifi-module
	net_context_p ctx = os_get_context();
	while(1) {
		while(if_net_decode(ctx) == 0);
		os_wait(40);
	}
}

void USART1_IRQHandler(void)
{
	uint8 c;
	USART_HandleTypeDef temp;
	temp.Instance = USART1;
	if(g_current_context != NULL){
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&g_current_context->handle, USART_FLAG_PE)) {
			//clear PE flag
			__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)&g_current_context->handle, USART_CLEAR_PEF);
			return;
		}
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)g_current_context, USART_FLAG_RXNE) != RESET) {
			gba_net_bufstream[gba_net_head++] = HAL_USART_READ(g_current_context->handle);
		}
		__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)g_current_context, USART_CLEAR_OREF | USART_CLEAR_TCF| USART_CLEAR_CTSF);
		if_pwr_set_interrupt_source(IF_INT_NET);
	} else {
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)&temp, USART_FLAG_RXNE) != RESET) {
			c = HAL_USART_READ(temp);
		}
		__HAL_USART_CLEAR_IT((USART_HandleTypeDef *)&temp, USART_CLEAR_OREF | USART_CLEAR_TCF| USART_CLEAR_CTSF);
	}
}

void if_net_reset(net_context_p ctx) {
	HAL_GPIO_WritePin(ctx->port, ctx->rst,GPIO_PIN_SET);
	if_delay(20);
	HAL_GPIO_WritePin(ctx->port, ctx->rst, GPIO_PIN_RESET);
	//GPIO_SetBits(ctx->port, ctx->rst);
	if_delay(20);
	HAL_GPIO_WritePin(ctx->port, ctx->rst, GPIO_PIN_SET);
}

void esp8266_init_config(net_context_p ctx) {
	GPIO_InitTypeDef GPIO_InitStructure;             
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_USART1_CONFIG(RCC_USART1CLKSOURCE_PCLK2);
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
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_NE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_FE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_ORE);
	__HAL_USART_ENABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_RXNE);
	NVIC_EnableIRQ(USART1_IRQn);
	GPIO_ResetBits(GPIOA, GPIO_PIN_3);				//power-up device
	GPIO_SetBits(GPIOB, GPIO_PIN_1);				//enable device
	if_net_reset(ctx);					//reset device
}

static void if_net_sendbyte(net_context_p ctx, uint8 b) {
	while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_TXE) == RESET);
   	//while (!(ctx->handle.Instance->SR & USART_FLAG_TXE));
   	//ctx->handle.Instance->DR = b;
	HAL_USART_WRITE(ctx->handle, b);
}

static void if_net_sendbytes(net_context_p ctx, uint8 * buffer, uint16 length) {
	uint16 i;
	for(i=0;i<length;i++) if_net_sendbyte(ctx, buffer[i]);
}

static void if_net_sendstring(net_context_p ctx, uint8_t * buffer) {
	uint16 len = strlen((char *)buffer);
	uint16 i;
	for(i=0;i<len;i++){
		if_net_sendbyte(ctx, buffer[i]);
	}
}

static uint16 if_net_compacting_net_buffer(net_context_p ctx, uint8 * response, uint16 length) {
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

static uint16 if_net_command(net_context_p ctx, uint8_t * command, uint8 * response, uint32 timeout) {
	uint32 i;
	uint8 c;
	uint16 len =0;
	uint16 state = 0;
	uint32 tickstart;
	gb_net_stat = NET_STAT_CLEAR;
	net_buffer_reset();
	if(command != NULL) {
		if_net_sendstring(ctx, command);
		if_net_sendstring(ctx, (uint8 *)"\r\n");
		gb_net_stat = NET_STAT_CLEAR;
	}
#if SHARD_RTOS_ENABLED
	g_current_context = ctx;
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
		while((HAL_GetTick() - tickstart) < timeout) { if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) break; }
		if((HAL_GetTick() - tickstart) >= timeout) goto exit_send_command;
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TC);
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_PE);
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_ERR);
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_IDLE);
		__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TXE);
		__HAL_USART_ENABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_RXNE);
		__HAL_UART_CLEAR_PEFLAG((UART_HandleTypeDef *)ctx);
		//NVIC_EnableIRQ(USART1_IRQn);
		while((HAL_GetTick() - tickstart) < timeout && (gb_net_stat & NET_STAT_BUF_RDY) == 0);// os_wait(1);
		
		while((HAL_GetTick() - tickstart) < timeout) {
			if(len == net_buffer_length()) break;				//no new data received
			else {
				len = net_buffer_length();
				gb_net_stat &= ~NET_STAT_BUF_RDY;		//clear net ready
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

static uint16 if_net_data(net_context_p ctx, uint8_t * data, uint16 length, uint8 * response, uint32 timeout) {
	uint32 i;
	uint8 c;
	uint16 len =0;
	uint16 state = 0;
	uint32 tickstart;
	gb_net_stat = NET_STAT_CLEAR;
	//timeout *= 100000;
	tickstart = HAL_GetTick();
	//__HAL_RCC_USART1_CLK_ENABLE();
	net_buffer_reset();
	if(data != NULL) {
		for(i=0;i<length;i++) if_net_sendbyte(ctx, data[i]);
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
	g_current_context = ctx;
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
		while((HAL_GetTick() - tickstart) < timeout && (gb_net_stat & NET_STAT_BUF_RDY) == 0);// os_wait(1);
		
		while((HAL_GetTick() - tickstart) < timeout) {
			if(len == net_buffer_length()) break;				//no new data received
			else {
				len = net_buffer_length();
				gb_net_stat &= ~NET_STAT_BUF_RDY;		//clear net ready
				timeout += 400;					//add another 200ms to timeout
				if_delay(400);						//keep waiting for another extra 200ms
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

static net_ssid_p if_net_ssid_create(net_context_p ctx, uint8 * buffer) {
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

void if_net_ssid_clear(net_ssid_p list) {
	net_ssid_p tlist = list;
	net_ssid_p plist;
	while(tlist != NULL) {
		plist = tlist;
		tlist = tlist->next;
		memset(plist, 0, sizeof(net_ssid));
		free(plist);
	}
}

net_ssid_p if_net_ssid_check(net_context_p ctx, uint8 * ssidname) {
	net_ssid_p iterator = ctx->ssid_list;
	if(ctx->ssid_list == NULL) return NULL;
	while(iterator != NULL) {
		if(strncmp((char *)iterator->name, (const char *)ssidname, IF_NET_MAX_SSID_NAME) == 0) return iterator;
		iterator = iterator->next;
	}
	return NULL;
}

uint8 if_net_get_ipconfig_async(net_context_p ctx) {
	uint16 len;
	uint8 * ptr;
	uint8 i, j;
	//get IP settings
	len = if_net_command(ctx, (uint8_t *)"AT+CIFSR", gba_net_bufstream, 200); 
	gba_net_bufstream[len] = 0;		//eos
	//check IP settings here (STAIP,"0.0.0.0")
	memset(ctx->ipv4, 0, 4);
	if((ptr = (uint8 *)strstr((char *)gba_net_bufstream, "+CIFSR:STAIP,")) != NULL) {
		ptr += 14;
		for(i=0;i<4;i++) {
			for(j=0;j<4;j++) {
				gba_net_bufstream[j] = *ptr++;
				switch(gba_net_bufstream[j]) {
					case '\"':
					case '.':
						gba_net_bufstream[j] = 0;		//eos
						ctx->ipv4[i] = atoi((const char *)gba_net_bufstream);
						j=4;
						break;
					default: break;
				}
			}
		}
	}
	return memcmp(ctx->ipv4, "\x0\x0\x0\x0", 4) != 0;
}


uint8 if_net_get_status_async(net_context_p ctx, net_status_p stat) {
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
	len = if_net_command(ctx, (uint8_t *)"AT+CIPSTATUS", gba_net_bufstream, 200); 
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

uint8 if_net_is_connected(net_context_p ctx, uint8 * ssidname) {
	uint16 len;
	int16 att;
	uint8 * ptr;
	uint8 i=0,j,k;
	uint8 ret = -1;
	uint8 cbuf[20];
	uint8 netbuf[2048];
	uint8 n1, n2, n3;
	if_net_wake(ctx);
	len = if_net_command(ctx, (uint8_t *)"AT+CWJAP?", netbuf, 200); 					//check joined access point
	netbuf[len] = 0;
	while(1) {
		if(strstr((char *)netbuf, "No AP")) {
			ctx->state &= ~IF_NET_STATE_CONNECTED;
			break;
		}
		if((ptr = (uint8 *)strstr((char *)netbuf, "+CWJAP:")) != NULL) {
			ptr += 8;
			ret = 0;
			ctx->state |= IF_NET_STATE_CONNECTED;
			i=0;
			if(ssidname != NULL) {
				while(*ptr != '\"' && i<(IF_NET_MAX_SSID_NAME - 1)) {
					ssidname[i++] = *ptr++;
				}
				//if(i == (IF_NET_MAX_SSID_NAME - 1)) return -1;
				ssidname[i] = 0;
			} else while(*ptr++ != '\"'); 
			//skip MAC
			while(*ptr++ != '\"');
			while(*ptr++ != '\"');
			for(i=0;i<6;i++) {
				n1 = *ptr++;
				n2 = *ptr++;
				n3 = *ptr++;
				//ctx->mac[i] = net_hex2byte(n1) << 4 | net_hex2byte(n2);
				if(n3 == '\"') break;
			}
			//while(*ptr++ != '\"');
			//skip channel
			while(*ptr++ != ',');
			i = 0;
			while((cbuf[i++] = *ptr++) != '\r');
			att = atoi((const char *)cbuf);
			ctx->att = att;
		}
		len = if_net_command(ctx, NULL, netbuf, 200); 
		netbuf[len] = 0;
		if(len == 0) {
			break;
		}
	}
	if_net_sleep(ctx);
	return ret;
}

uint16 if_net_wait_rdy(net_context_p ctx, uint32 timeout) {
	OS_DEBUG_ENTRY(if_net_wait_rdy);
	uint32 i;
	uint16 ret = -1;
	uint32 tickstart = HAL_GetTick();
	uint32 curtick;
	//enable interrupt (do not waste any received data)
	//NVIC_EnableIRQ(USART1_IRQn);
	gb_net_stat &= ~NET_STAT_BUF_RDY;		//buffer ready
	while(((curtick = HAL_GetTick()) - tickstart) < timeout) { 
		if( (gb_net_stat & NET_STAT_BUF_RDY) != 0) {
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

uint8 if_net_ssid_list_async(net_context_p ctx) {
	OS_DEBUG_ENTRY(if_net_ssid_list_async);
	uint16 len;
	uint16 i = 0,j;
	uint8 num_ssid = 0;
	uint8 cbuf[8];
	uint16 plen;
	uint8 state = 0;
	uint8 mark[] = "+CWLAP:";
	net_ssid_p clist = NULL;		//current list
	net_ssid_p plist = NULL;		//previous list
	net_ssid_p tlist;
	//uint8 ssidname[256];
	uint32 timeout = 5000;
	NET_LOCK(ctx);
	if_net_wake(ctx);
	plen = if_net_command(ctx, (uint8 *)"AT+CWLAP", gba_net_bufstream, 8000); 
	len = plen;
#if 0
	while(plen != 0) {
		//some delay here (please do something)
		if(if_net_wait_rdy(ctx, timeout) != 0) break;
		plen = if_net_data(ctx, NULL, 0, gba_net_bufstream + len, 200); 
		len += plen;
		timeout = 1000;
	}
#endif
#if 1
	i=0;
	while(i<len) {
		if(gba_net_bufstream[i] == mark[state]) state++;
		else state = 0;
		if(state == 7) {			//matched
			tlist = if_net_ssid_create(ctx, gba_net_bufstream + i + 1);
			if(tlist == NULL) break;		//unable to create ssid
			if(clist == NULL) clist = tlist;
			else plist->next = tlist;
			plist = tlist;
			
			while(gba_net_bufstream[i++] != ',');
			while(gba_net_bufstream[i++] != ',');
			j=0;
			while((cbuf[j++] = gba_net_bufstream[i++]) != ',');
			tlist->att = atoi((const char *)cbuf);
			num_ssid++;
		}
		i++;
	}
	if(clist != NULL) {
		//check if ssid_list is not empty
		if(ctx->ssid_list != NULL) if_net_ssid_clear(ctx->ssid_list);
		//set current context ssid list
		ctx->ssid_list = clist;
	}
#endif
	if_net_sleep(ctx);
	NET_UNLOCK(ctx);
	OS_DEBUG_EXIT();
	return num_ssid;
}

uint8 if_net_ssid_join_async(net_context_p ctx, uint8 * ssidname, uint8 * username, uint8 * password) {
	OS_DEBUG_ENTRY(if_net_ssid_join_async);
	char cbuf[128];
	uint8 ret = 1;			//unable to connect (wrong password)
	uint16 len;
	uint8 * ptr;
	uint8 i, j;
	//uint8 netbuf[256];
	//AT+CWDHCP=<mode>,<en> 
	NET_LOCK(ctx);
	if_net_wake(ctx);
	len = if_net_command(ctx, (uint8 *)"AT+CWDHCP_CUR=1,1", gba_net_bufstream, 200); 
	sprintf(cbuf, "AT+CWJAP=\"%s\",\"%s\"", ssidname, password);
	//sprintf(cbuf, "AT+CWJAP_DEF=\"%s\",\"%s\"", "wikanto_internet", "herupoetranto");
	//some delay here (please do something)
	ctx->state &= ~IF_NET_STATE_CONNECTED;
	while(1) {
		len = if_net_command(ctx, (uint8_t *)cbuf, gba_net_bufstream, 4000); 
		gba_net_bufstream[len] = 0;
		if(strstr((char *)gba_net_bufstream, "WIFI CONNECTED") != NULL) ctx->state |= IF_NET_STATE_CONNECTED;
		if(strstr((char *)gba_net_bufstream, "WIFI GOT IP") != NULL) ctx->state |= IF_NET_STATE_IP_ASSIGNED;
		if(strstr((char *)gba_net_bufstream, "FAIL") != NULL) break;
		if(strstr((char *)gba_net_bufstream, "OK") != NULL) break;
		if(len == 0) break;			//timeout
	}
	if(ctx->state & IF_NET_STATE_CONNECTED) {
		//AT+CWAUTOCONN
		//do {
		len = if_net_command(ctx, (uint8_t *)"AT+CWAUTOCONN=1", gba_net_bufstream, 200);
		gba_net_bufstream[len] = 0;		//eos
		//} while ((ptr = (uint8 *)strstr((char *)netbuf, "busy")) != NULL) ;
		//get ip configuration
		if_net_get_ipconfig_async(ctx);
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
	if_net_sleep(ctx);
	NET_UNLOCK(ctx);
	OS_DEBUG_EXIT();
	return ret;
}

uint16 if_net_escape_string(uint8 * payload, uint16 length, uint8 * escaped) {
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

net_conn_p if_net_tcp_open_async(net_context_p ctx, uint8 * host, uint16 port, uint16 mode) {
	OS_DEBUG_ENTRY(if_net_tcp_open_async);
	uint16 len;
	uint8 cbuf[500];
	uint8 ip4ddr[4];
	uint8 ipstr[20];
	net_conn_p conn;
	uint8 retry = 3;
	//if_net_command(ctx, (uint8 *)"AT+CIPMUX=0", cbuf, 200); 				//single connection
	//goto use_hardware_dns;
	if(mode & NET_LISTEN && g_server_channel != NULL) goto exit_tcp_connect;		//server full
	use_system_dns:
	if(net_is_ip4ddr(host) != 0) {
		//try netbios
		if(netbios_translate(ctx, host, ip4ddr) == 0) goto use_ip_connect;
		goto use_hardware_dns;
		//try system DNS
		if(dns_translate(ctx, host, ip4ddr) != 0) goto use_hardware_dns;			//try translate domain name address to ip address
		use_ip_connect:
		sprintf((char *)ipstr, "%d.%d.%d.%d", ip4ddr[0], ip4ddr[1], ip4ddr[2], ip4ddr[3]);
		if((mode & NET_LISTEN) != 0) {
			conn = net_conn_create(ctx, NET_TYPE_TCP | mode, 5000, port, NULL, 4096, NULL);
		} else {
			conn = net_conn_create(ctx, NET_TYPE_TCP | mode, 5000, port, NULL, 8192, NULL);
		}
		if(conn == NULL) goto exit_tcp_connect;		//no channel available
		if(mode & NET_LISTEN) {
			g_server_channel = conn;
			sprintf((char *)cbuf, "AT+CIPSERVER=1,%d", port);
		} else {
#if NET_USE_CHANNEL
			sprintf((char *)cbuf, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d", conn->id, ipstr, port);
#else
			sprintf((char *)cbuf, "AT+CIPSTART=\"TCP\",\"%s\",%d", ipstr, port);
#endif
		}
	} else {
		use_hardware_dns:
		if((mode & NET_LISTEN) != 0) {
			conn = net_conn_create(ctx, NET_TYPE_TCP | mode, 5000, port, NULL, 4096, NULL);
		} else {
			conn = net_conn_create(ctx, NET_TYPE_TCP | mode, 5000, port, NULL, 8192, NULL);
		}
		if(conn == NULL) goto exit_tcp_connect;		//no channel available
		if(mode & NET_LISTEN) {
			g_server_channel = conn;
			sprintf((char *)cbuf, "AT+CIPSERVER=1,%d", port);
		} else {
#if NET_USE_CHANNEL
			sprintf((char *)cbuf, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d", conn->id, host, port);
#else
			sprintf((char *)cbuf, "AT+CIPSTART=\"TCP\",\"%s\",%d", host, port);
#endif
		}
	}
	len = if_net_command(ctx, (uint8 *)cbuf, gba_net_bufstream, 5000); 
	gba_net_bufstream[len] = 0;
	if(strstr((char *)gba_net_bufstream, "ERROR") != NULL) {
		//error connecting to server
		//error connecting to server
		retry_connection:
		if(conn != NULL) { 
			net_conn_close(conn);
			if(mode & NET_LISTEN) g_server_channel = NULL;	//failed to open server port
			conn = NULL;
		}
		if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->exception_handler != NULL) 
			ctx->exception_handler(ctx, "unable to connect");
		//goto exit_tcp_connect;
		if(retry == 0) goto exit_tcp_connect;
		retry--;
		goto use_hardware_dns;
	}
	if(strstr((char *)gba_net_bufstream, "OK") != NULL) {
		//ready connect
		connect_ready:
#if ESP8266_TRANSPARENT_TRANSMISSION
		//use transparent connection
		if_net_command(ctx, (uint8 *)"AT+CIPMODE=1", cbuf, 200); 				//single connection
		if_delay(20);
#endif
		if((mode & NET_LISTEN) == 0)
			ctx->state |= IF_NET_STATE_OPENED;
		//ret = 0;
	}
	exit_tcp_connect:
	OS_DEBUG_EXIT();
	return conn;
}

void if_net_flush_async(net_conn_p conn) {
	char buffer[30];
	if(conn == NULL) return;
#if NET_USE_CHANNEL
	sprintf(buffer, "AT+CIPCLOSE=%d\r\n", conn->id);
#else
	sprintf(buffer, "AT+CIPCLOSE\r\n");
#endif
	if_net_sendstring(conn->netctx, (uint8 *)buffer);
}

void if_net_tcp_close_async(net_conn_p conn) {
	OS_DEBUG_ENTRY(if_net_tcp_close_async);
	uint32 cntr = 0;
	char buffer[30];
	if(conn == NULL) return;
	if(conn->state != NET_CONN_CLOSED) {
#if ESP8266_TRANSPARENT_TRANSMISSION
		if_net_sendstring(ctx, (uint8 *)"+++");
#endif
		if(conn->base.mode & NET_LISTEN) {
			sprintf(buffer, "AT+CIPSERVER=0,%d\r\n", conn->port);
			g_server_channel = NULL;			//release server channel
		} else {
#if NET_USE_CHANNEL
			sprintf(buffer, "AT+CIPCLOSE=%d\r\n", conn->id);
#else
			sprintf(buffer, "AT+CIPCLOSE\r\n");
#endif
		}
		if_net_sendstring(conn->netctx, (uint8 *)buffer);
		if((conn->base.mode & NET_LISTEN) == 0) {
			conn->netctx->state &= ~IF_NET_STATE_OPENED;		//switch state
		}
		conn->state = NET_CONN_CLOSED;
		net_conn_close(conn);
		net_buffer_reset();
	}
	OS_DEBUG_EXIT();
}


uint16 if_net_tcp_send_async(net_conn_p conn, uint8 * payload, uint16 length) {
	OS_DEBUG_ENTRY(if_net_tcp_send_async);
	uint8 lbuf[500];
	//uint8 pbuf[512];
	uint16 len;
	uint16 i;
	//uint32 wait = 200 * 1000;
	//start send request
#if ESP8266_TRANSPARENT_TRANSMISSION
	if_net_sendbytes(ctx, payload, length);
#else
#if NET_USE_CHANNEL
	sprintf((char *)lbuf, "AT+CIPSEND=%d,%d", conn->id, length);
#else
	sprintf((char *)lbuf, "AT+CIPSEND=%d", length);
#endif
	if_net_command(conn->netctx, (uint8 *)lbuf, gba_net_bufstream, 200);
	if_net_data(conn->netctx, payload, length, NULL, 0);
#endif
	OS_DEBUG_EXIT();
	return length;
}

uint16 if_net_tcp_recv_async(net_conn_p conn, uint8 * response, uint16 bufsz) {
	OS_DEBUG_ENTRY(if_net_tcp_recv_async);
	uint16 plen;
	uint16 len;
	uint16 i = 0;
	len = 0;
	uint16 data_left;
	uint16 timeout = 5000;
#if ESP8266_TRANSPARENT_TRANSMISSION
	len = if_net_command(ctx, NULL, response, 8000); 
#else
	//goto try_next;
	//while(1) {
	//plen = if_net_data(conn->netctx, NULL, 0, response, timeout); 
	len = (bufsz < conn->base.buflen)?bufsz:conn->base.buflen;
	//data_left = conn->base.buflen - len;
	memcpy(response, conn->base.buffer, len);
	//if(data_left == 0) {
	conn->base.buflen = 0;
	conn->base.bufend = 0;
	//	conn->state &= ~NET_CONN_BUFRDY;		//clear ready flag
	//}
	//len = if_net_compacting_net_buffer(ctx, response, plen);
#endif
	exit_tcp_recv:
	OS_DEBUG_EXIT();
	return len;
}

//transmit tcp request and wait for response
uint16 if_net_tcp_transmit_async(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	OS_DEBUG_ENTRY(if_net_tcp_transmit);
	uint16 len = 0;
	net_conn_p conn;
	//try to connect in-case wakeup from sleep
	if((ctx->state & IF_NET_STATE_CONNECTED) == 0) {		//not connected
		if_net_try_connect_async(ctx);
	}
	if((ctx->state & IF_NET_STATE_CONNECTED) != 0) {			//not connected
		NET_LOCK(ctx);
		if((conn = if_net_tcp_open_async(ctx, host, port, NET_TRANSMIT)) == NULL)  {
			//error connecting to server
			//NET_UNLOCK(ctx);
			goto exit_tcp_transmit;
		}
		//set receive buffer to global net buffer
		if_net_set_buffer(conn, gba_net_buffer, 16384);
		//start transmit
		if_net_tcp_send_async(conn, payload, length);
		len = 0;
		if(if_net_tcp_accept(conn) == 0) {
			len = if_net_tcp_recv_async(conn, response, conn->base.bufsz);
		}
		exit_tcp_transmit:
		//close connection
		if_net_tcp_close_async(conn);
		NET_UNLOCK(ctx);
	}
	OS_DEBUG_EXIT();
	return len;		//unable to found +IPD prefix
}

net_conn_p if_net_udp_open_async(net_context_p ctx, uint8 * host, uint16 port, uint16 mode) {
	OS_DEBUG_ENTRY(if_net_udp_open_async);
	uint16 len;
	net_conn_p conn = NULL;
	uint8 cbuf[500];
	uint8 ip4ddr[4];
	uint8 ipstr[20];
	if(net_is_ip4ddr(host) != 0) {
		//try netbios
		if(netbios_translate(ctx, host, ip4ddr) == 0) goto use_ip_connect;
		goto use_hardware_dns;
		//try system DNS
		if(dns_translate(ctx, host, ip4ddr) != 0) goto use_hardware_dns;			//try translate domain name address to ip address
		use_ip_connect:
		sprintf((char *)ipstr, "%d.%d.%d.%d", ip4ddr[0], ip4ddr[1], ip4ddr[2], ip4ddr[3]);
		conn = net_conn_create(ctx, NET_TYPE_UDP | mode, 3000, port, NULL, 1460, NULL);
		if(conn == NULL) goto exit_udp_connect;		//no channel available
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
		use_hardware_dns:
		conn = net_conn_create(ctx, NET_TYPE_UDP | mode, 3000, port, NULL, 1460, NULL);
		if(conn == NULL) goto exit_udp_connect;		//no channel availabl
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
	}
	len = if_net_command(ctx, (uint8 *)cbuf, gba_net_bufstream, 3000); 
	gba_net_bufstream[len] = 0;
	if(strstr((char *)gba_net_bufstream, "ERROR") != NULL) {
		goto retry_connection;
	}
	if(strstr((char *)gba_net_bufstream, "OK") != NULL) goto connect_ready;
	//try waiting until ready
	len = if_net_command(ctx, (uint8 *)NULL, gba_net_bufstream, 5000); //wait till ready
	gba_net_bufstream[len] = 0;
	if(strstr((char *)gba_net_bufstream, "ERROR") != NULL) {
		retry_connection:
		if(conn != NULL) {
			net_conn_close(conn);
			conn = NULL;
		}
		if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->exception_handler != NULL) 
			ctx->exception_handler(ctx, "unable to connect");
		//error connecting to server
		goto exit_udp_connect;
	}
	if(strstr((char *)gba_net_bufstream, "OK") != NULL) {
		//ready connect
		connect_ready:
#if 0//ESP8266_TRANSPARENT_TRANSMISSION
		//use transparent connection
		//if_net_command(ctx, (uint8 *)"ATE0", cbuf, 200); 				//timeout 5 second
		if_net_command(ctx, (uint8 *)"AT+CIPMODE=1", cbuf, 200); 				//single connection
		if_delay(20);
#endif
		if((mode & NET_LISTEN) == 0)
			ctx->state |= IF_NET_STATE_OPENED;
	}
	exit_udp_connect:
	OS_DEBUG_EXIT();
	return conn;
}

void if_net_udp_close_async(net_conn_p conn) {
	OS_DEBUG_ENTRY(if_net_udp_close_async);
	uint32 cntr = 0;
	char buffer[30];
	if(conn == NULL) return;
	if(conn->state != 0) {
#if 0//ESP8266_TRANSPARENT_TRANSMISSION
		//if_net_sendstring(ctx, (uint8 *)"+++");
		if_net_sendstring(ctx, (uint8 *)"AT+CIPMODE=0\r\n");
		if(ctx->state & IF_NET_STATE_CRITICAL_SECTION) {
			for(cntr=0;cntr<2000000;cntr++);
		} else if_delay(300);
#endif
#if NET_USE_CHANNEL
		sprintf(buffer, "AT+CIPCLOSE=%d\r\n", conn->id);
#else
		sprintf(buffer, "AT+CIPCLOSE\r\n");
#endif
		if_net_sendstring(conn->netctx, (uint8 *)buffer);
		if((conn->base.mode & NET_LISTEN) == 0) {
			conn->netctx->state &= ~IF_NET_STATE_OPENED;		//switch state
		}
		conn->state = NET_CONN_CLOSED;
		net_conn_close(conn);
		net_buffer_reset();
	}
	OS_DEBUG_EXIT();
}

uint16 if_net_udp_send_async(net_conn_p conn, uint8 * payload, uint16 length, uint8 addr[], uint16 port) {
	OS_DEBUG_ENTRY(if_net_udp_send_async);
	uint8 lbuf[500];
	uint16 len;
	uint16 i;
	//start send request
#if ESP8266_TRANSPARENT_TRANSMISSION
	if_net_sendbytes(ctx, payload, length);
#else
#if NET_USE_CHANNEL
	sprintf((char *)lbuf, "AT+CIPSEND=%d,%d", conn->id, length);
#else
	sprintf((char *)lbuf, "AT+CIPSEND=%d", length);
#endif
	if(addr != NULL) {
		sprintf((char *)lbuf, "%s,\"%d.%d.%d.%d\",%d", lbuf, addr[0], addr[1], addr[2], addr[3], port);
	}
	if_net_command(conn->netctx, (uint8 *)lbuf, gba_net_bufstream, 200);
	if_net_data(conn->netctx, payload, length, NULL, 0);
#endif
	OS_DEBUG_EXIT();
	return length;
}

uint16 if_net_udp_recv_async(net_conn_p conn, uint8 * response, uint16 bufsz) {
	OS_DEBUG_ENTRY(if_net_udp_recv_async);
	uint16 plen;
	uint16 len;
	uint16 i = 0;
	uint8 clen[10];
	uint8 * nextptr;
	uint8 * ptr;
	uint16 timeout = 5000;
	len = 0;
	nextptr = response;
#if 0//ESP8266_TRANSPARENT_TRANSMISSION
	len = if_net_command(ctx, NULL, response, 5000); 
#else
	//len = if_net_data(conn->netctx, NULL, 0, response, timeout); 
	len = (bufsz < conn->base.buflen)?bufsz:conn->base.buflen;
	memcpy(response, conn->base.buffer, len);
	conn->base.buflen = 0;
	conn->base.bufend = 0;
	//len = if_net_compacting_net_buffer(ctx, response, len);
#endif
	exit_udp_recv:
	OS_DEBUG_EXIT();
	return len;
}

//transmit udp reqeuest and wait for response
uint16 if_net_udp_transmit_async(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	OS_DEBUG_ENTRY(if_net_udp_transmit_async);
	uint16 len;
	uint8 retry = 3;
	net_conn_p conn;
	//try to connect in-case wakeup from sleep
	if((ctx->state & IF_NET_STATE_CONNECTED) == 0) {		//not connected
		if_net_try_connect_async(ctx);
	}
	if((ctx->state & IF_NET_STATE_CONNECTED) != 0) {		//not connected
		NET_LOCK(ctx);
		if((conn = if_net_udp_open_async(ctx, host, port, NET_TRANSMIT)) == NULL)  {
			//error connecting to server
			NET_UNLOCK(ctx);
			goto exit_udp_transmit;
		}
		//start transmit
		retry_transmit:
		//if_net_escape_string(payload, length, response);
		if_net_udp_send_async(conn, payload, length, NULL, 0);
		len = 0;
		if(if_net_udp_accept(conn) == 0) {
			len = if_net_udp_recv_async(conn, response, 1400);
			if(len == 0 && retry != 0) {
				retry--;
				goto retry_transmit;
			}
		}
		if_net_udp_close_async(conn);
		NET_UNLOCK(ctx);
	}
	exit_udp_transmit:
	OS_DEBUG_EXIT();
	return len;		//unable to found +IPD prefix
}

static void if_net_tick(net_context_p ctx) {
	uint8 ssidname[256];
	if(NET_BUSY(ctx)) return;
	//if(if_net_is_connected(ctx, ssidname) == 0) if_timer_set_tick(28);
	//else if_timer_set_tick(7);
}

void if_net_wake(net_context_p ctx) {
	OS_DEBUG_ENTRY(if_net_wake);
	if((ctx->state & IF_NET_STATE_SLEEP) != 0 || ctx->cb_depth == 0) {
		if(ctx->prepare_callback != NULL) ctx->prepare_callback(ctx->p_ctx);
		ctx->state &= ~IF_NET_STATE_SLEEP;
	} 
	ctx->cb_depth++;
	OS_DEBUG_EXIT();
}

uint8 if_net_sleep_async(net_context_p ctx, uint8 mode) {
	if_net_command(ctx, (uint8 *)"AT+SLEEP=1", NULL, 200);
}

void if_net_power_up(net_context_p ctx) {
	//GPIO_ResetBits(GPIOA, GPIO_PIN_3);				//power-up device (vcc)
	//GPIO_SetBits(GPIOB, GPIO_PIN_1);				//enable device (chip_en) 
#if SHARD_RTOS_ENABLED 
	while(NET_BUSY(ctx)) { if_delay(100); }
#endif
	if_net_reset(ctx);
}

uint8 if_net_power_down(net_context_p ctx) {
	//GPIO_ResetBits(GPIOB, GPIO_PIN_1);				//disable device (chip_en)
	//GPIO_SetBits(GPIOA, GPIO_PIN_3);				//power-down device (vcc)
	uint8 ret = 1;
#if SHARD_RTOS_ENABLED 
	while(NET_BUSY(ctx)) { if_delay(100); }
#else 
	if(NET_BUSY(ctx)) return;
#endif
	if(g_server_channel == NULL) {
		ret = 0;
		if_net_command(ctx, (uint8 *)"AT+GSLP=1000", NULL, 200); 				//deepsleep
	} else {
		if_net_command(ctx, (uint8 *)"AT+SLEEP=1", NULL, 200);
	}
	return ret;
}

void if_net_deep_sleep(net_context_p ctx) {
	uint8 netbuf[256];
	//sprintf((char *)netbuf, "AT+GLSP=%d", 2000);
	//if_net_command(ctx, netbuf, netbuf, 200); 				//timeout 5 second
}

#if SHARD_RTOS_ENABLED 

struct net_async_params {
	uint8 type;
	void * param1;
	void * param2;
	void * param3;
	void * param4;
	void * param5;
	void * param6;
	void * param7;
} net_async_params;

uint8 if_net_get_status(net_context_p ctx, net_status_p stats) {
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	req.type = 6;
	req.param1 = stats;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_net_get_ipconfig(net_context_p ctx) {
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	req.type = 5;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
}

void if_net_sleep(net_context_p ctx) {
	OS_DEBUG_ENTRY(if_net_sleep);
	uint8 ret;
	struct net_async_params req;
	os_message * msg;
	--ctx->cb_depth;
	if((ctx->state & IF_NET_STATE_SLEEP) == 0 && ctx->cb_depth == 0) {
		if(ctx->finish_callback != NULL) ctx->finish_callback(ctx->f_ctx);
		ctx->state |= IF_NET_STATE_SLEEP;
		req.type = 4;
		req.param1 = (void *)1;
		msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
		os_delete_message(msg);
	}
	OS_DEBUG_EXIT();
}

uint8 if_net_ssid_join(net_context_p ctx, uint8 * ssidname, uint8 * username, uint8 * password) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0;
	req.param1 = ssidname;
	req.param2 = password;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_net_ssid_list(net_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	req.type = 1;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &num_ssid));
	os_delete_message(msg);
	return num_ssid;
}

uint8 if_net_try_connect(net_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 buffer[16];
	uint8 ret;
	req.type = 0x1E;
	req.param1 = NULL;
	req.param2 = (void *)(unsigned)sizeof(buffer);
	req.param3 = buffer;
	req.param4 = (void *)(unsigned)sizeof(buffer);
	req.param5 = buffer;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

net_conn_p if_net_tcp_open (net_context_p ctx, uint8 * host, uint16 port, uint16 mode) {
	struct net_async_params req;
	os_message * msg;
	net_conn_p ret;
	req.type = 0x20;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = (void *)(unsigned)mode;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_tcp_send(net_conn_p conn, uint8 * payload, uint16 length) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x21;
	req.param1 = conn;
	req.param2 = payload;
	req.param3 = (void *)(unsigned)length;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_tcp_accept(net_conn_p conn) {
	OS_DEBUG_ENTRY(if_net_tcp_accept);
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

uint16 if_net_tcp_recv(net_conn_p conn, uint8 * response, uint16 bufsz) {
	OS_DEBUG_ENTRY(if_net_tcp_recv);
	uint16 len = 0;
	uint16 timeout = conn->timeout;
	if(if_net_tcp_accept(conn) == 0) {
		len = (bufsz < conn->base.buflen)?bufsz:conn->base.buflen;
		memcpy(response, conn->base.buffer, len);
		conn->base.buflen = 0;
		conn->base.bufend = 0;
	}
	exit_tcp_recv:
	OS_DEBUG_EXIT();
	return len;
	
#if 0
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x22;
	req.param1 = conn;
	req.param2 = response;
	req.param3 = (void *)(unsigned)bufsize;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
#endif
}

void if_net_tcp_close(net_conn_p conn) {
	struct net_async_params req;
	os_message * msg;
	void * ret;
	req.type = 0x23;
	req.param1 = conn;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	//return ret;
}

uint16 if_net_tcp_transmit (net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x24;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

net_conn_p if_net_udp_open (net_context_p ctx, uint8 * host, uint16 port, uint16 mode) {
	struct net_async_params req;
	os_message * msg;
	net_conn_p ret;
	req.type = 0x28;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = (void *)(unsigned)mode;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_udp_send (net_conn_p conn, uint8 * payload, uint16 length, uint8 addr[], uint16 port) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x29;
	req.param1 = conn;
	req.param2 = payload;
	req.param3 = (void *)(unsigned)length;
	req.param4 = addr;
	req.param5 = (void *)(unsigned)port;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_udp_accept(net_conn_p conn) {
	OS_DEBUG_ENTRY(if_net_udp_accept);
	uint32 i;
	uint16 ret = -1;
	uint32 timeout = conn->timeout;
	uint32 tickstart = HAL_GetTick();
	uint32 curtick;
	conn->state |= NET_CONN_READY;
	while(((curtick = HAL_GetTick()) - tickstart) < timeout) { 
		if( (conn->state & NET_CONN_BUFRDY) != 0) {
			ret = 0;
			conn->state &= ~NET_CONN_BUFRDY;		//clear ready flag
			goto exit_udp_accept;
		}
		os_wait(40);
	}
	exit_udp_accept:
	OS_DEBUG_EXIT();
	return ret;
}

uint16 if_net_udp_recv(net_conn_p conn, uint8 * response, uint16 bufsz) {
	OS_DEBUG_ENTRY(if_net_udp_recv);
	uint16 len = 0;
	if(if_net_udp_accept(conn) == 0) {
		len = (bufsz < conn->base.buflen)?bufsz:conn->base.buflen;
		memcpy(response, conn->base.buffer, len);
		conn->base.buflen = 0;
		conn->base.bufend = 0;
	}
	exit_udp_recv:
	OS_DEBUG_EXIT();
	return len;
	
#if 0
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x2A;
	req.param1 = conn;
	req.param2 = response;
	req.param3 = (void *)(unsigned)bufsize;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
#endif
}

void if_net_udp_close (net_conn_p conn) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x2B;
	req.param1 = conn;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(conn, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
}

uint16 if_net_udp_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x2C;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}


ssl_handle_p if_net_ssl_open(net_context_p ctx, uint8 * host, uint16 port, uint16 mode, ssl_cert_p cert, ssl_keys_p keys) {
	struct net_async_params req;
	os_message * msg;
	ssl_handle_p ret;
	req.type = 0x30;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = (void *)(unsigned)mode;
	req.param4 = cert;
	req.param5 = keys;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_ssl_send(ssl_handle_p ctx, uint8 * payload, uint16 length) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x31;
	req.param1 = ctx;
	req.param1 = payload;
	req.param2 = (void *)(unsigned)length;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_ssl_recv(ssl_handle_p ctx, uint8 * response, uint16 bufsize) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x32;
	req.param1 = ctx;
	req.param1 = response;
	req.param2 = (void *)(unsigned)bufsize;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

void if_net_ssl_close(ssl_handle_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x33;
	req.param1 = ctx;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
}

uint16 if_net_ssl_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response, ssl_cert_p cert) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 0x34;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	req.param6 = cert;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

void if_net_task() {
	#define TICK_LIMIT(x)		(x*5)
	OS_DEBUG_ENTRY(if_net_task);
	os_message * msg;
	net_context_p ctx = os_get_context();
	uint16 tick = 3;
	uint16 len;
	int32 timeout = 200;
	uint8 buffer[50];
	void * ret;
	struct net_async_params * params;
	while(1) {
		timeout = 200;
		msg = os_dequeue_message();
		if(msg != NULL) {
			ctx = msg->context;
			if(ctx == NULL) goto abort_operation;
			if(msg->reqlen == 0) goto abort_operation;
			params = msg->request;
			switch(params->type) {
				case 0:		//join
					((uint8 *)msg->response)[0] = if_net_ssid_join_async(ctx, params->param1, params->param2, params->param3);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 1:		//list
					((uint8 *)msg->response)[0] = if_net_ssid_list_async(ctx);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 4:		//sleep
					((uint8 *)msg->response)[0] = if_net_sleep_async(ctx, (uint8)params->param1);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 5:		//get ipconfig
					((uint8 *)msg->response)[0] = if_net_get_ipconfig_async(ctx);
					msg->reslen = 0;
					timeout = 0;
					break;
				case 6:		//get status
					((uint8 *)msg->response)[0] = if_net_get_status_async(ctx, params->param1);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x1E:		//check connection
					len = if_net_try_connect_async(ctx);
					if(len == 0) ctx->state |= IF_NET_STATE_CONNECTED;
					else ctx->state &= ~IF_NET_STATE_CONNECTED;
					((uint8 *)msg->response)[0] = len;
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x1F:
					if_net_flush_async(params->param1);
					((uint16 *)msg->response)[0] = 0;
					msg->reslen = 0;
					timeout = 0;
					break;
				
				//TCP async APIs
				case 0x20:
					if(timeout != 0) os_wait(timeout);
					((net_context_p *)msg->response)[0] = NULL;
					if((ret = (void *)if_net_tcp_open_async(ctx, params->param1, (uint16)params->param2, (uint16)params->param3)) != 0) {
						((net_conn_p *)msg->response)[0] = ret;
					}
					msg->reslen = sizeof(net_context_p);	
					timeout = 0;
					break;
				case 0x21:
					((uint16 *)msg->response)[0] = if_net_tcp_send_async(params->param1, params->param2, (uint16)params->param3);
					msg->reslen = sizeof(uint16);	
					timeout = 0;
					break;
				case 0x22:
					((uint16 *)msg->response)[0] = 0;
					if(if_net_tcp_accept(params->param1) == 0) {
						len = if_net_tcp_recv_async(params->param1, gba_net_buffer, (uint16)params->param3);
						if(len < (uint16)params->param3) {
							memcpy(params->param2, gba_net_buffer, len);
							((uint16 *)msg->response)[0] = len;
						}
					}
					msg->reslen = sizeof(uint16);	
					timeout = 0;
					break;
				case 0x23:
					if_net_tcp_close_async(params->param1);
					((uint16 *)msg->response)[0] = 0;
					msg->reslen = sizeof(uint16);	
					timeout = 2000;
					break;
				case 0x24:		//tcp transmit
					if(timeout != 0) os_wait(timeout);
					((uint16 *)msg->response)[0] = if_net_tcp_transmit_async(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5);
					msg->reslen = sizeof(uint16);	
					timeout = 2000;
					break;
				
				//UDP async APIs
				case 0x28:
					if(timeout != 0) os_wait(timeout);
					((net_context_p *)msg->response)[0] = NULL;
					if((ret = if_net_udp_open_async(ctx, params->param1, (uint16)params->param2, (uint16)params->param3)) != NULL) {
						((net_conn_p *)msg->response)[0] = ret;
					}
					msg->reslen = sizeof(net_context_p);	
					timeout = 0;
					break;
				case 0x29:
					((uint16 *)msg->response)[0] = if_net_udp_send_async(params->param1, params->param2, (uint16)params->param3, params->param4, (uint16)params->param5);
					msg->reslen = sizeof(uint16);	
					timeout = 0;
					break;
				case 0x2A:
					((uint16 *)msg->response)[0] = 0;
					if(if_net_udp_accept(params->param1) == 0) {
						len = if_net_udp_recv_async(params->param1, gba_net_buffer, (uint16)params->param3);
						if(len < (uint16)params->param3) {
							memcpy(params->param2, gba_net_buffer, len);
							((uint16 *)msg->response)[0] = len;
						}
					}
					msg->reslen = sizeof(uint16);
					timeout = 0;
					break;
				case 0x2B:
					if_net_udp_close_async(params->param1);
					((uint16 *)msg->response)[0] = 0;
					msg->reslen = sizeof(uint16);	
					timeout = 2000;
					break;
				case 0x2C:		//udp transmit
					((uint16 *)msg->response)[0] = if_net_udp_transmit_async(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5);
					msg->reslen = sizeof(uint16);
					timeout = 2000;
					break;
				
				//SSL async APIs
				case 0x30:		//ssl connect
					if(timeout != 0) os_wait(timeout);
					((ssl_handle_p *)msg->response)[0] = if_ssl_open_async(ctx, params->param1, (uint16)params->param2, (uint16)params->param3, params->param4, params->param5);
					msg->reslen = sizeof(ssl_handle_p);
					timeout = 0;
					break;
				case 0x31:		//ssl send
					((uint16 *)msg->response)[0] = if_ssl_send_async(params->param1, params->param2, (uint16)params->param3);
					msg->reslen = sizeof(uint16);	
					timeout = 0;
					break;
				case 0x32:		//ssl recv
					((uint16 *)msg->response)[0] = 0;
					len = if_ssl_recv_async(params->param1, gba_net_buffer, (uint16)params->param3);
					if(len < (uint16)params->param3) {
						memcpy(params->param2, gba_net_buffer, len);
						((uint16 *)msg->response)[0] = len;
					}
					msg->reslen = sizeof(uint16);
					timeout = 0;
					break;
				case 0x33:			//ssl close
					((uint16 *)msg->response)[0] = 0;
					if_ssl_close_async(params->param1) ;
					msg->reslen = sizeof(uint16);
					timeout = 2000;
					break;
				case 0x34:		//ssl transmit
					if(timeout != 0) os_wait(timeout);
					((uint16 *)msg->response)[0] = if_ssl_transmit_async(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5, params->param6);
					msg->reslen = sizeof(uint16);
					timeout = 2000;
					break;
				//case 0x35:		//ssl init (certificate)
				//	((uint16 *)msg->response)[0] = if_ssl_init_async(ctx, params->param1, (uint16)params->param2);
				//	msg->reslen = sizeof(uint16);
				//	timeout = 0;
				//	break;
				//case 0x36:		//ssl release
				//	((uint16 *)msg->response)[0] = 0;
				//	if_ssl_release_async(ctx) ;
				//	msg->reslen = sizeof(uint16);
				//	timeout = 0;
				//	break;
				
#if SHARD_BLE_MODULE == MOD_ESP32
				case 0x80:			//list device
					((uint8 *)msg->response)[0] = if_ble_dev_list_async(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x81:			//connect			
					((uint8 *)msg->response)[0] = if_ble_connect_async(msg->context, params->param1);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x82:		//disconnect
					((uint8 *)msg->response)[0] = if_ble_disconnect_async(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x8A:		//read
					((uint16 *)msg->response)[0] = if_ble_read_async(msg->context, params->param1, (uint16)params->param2);
					msg->reslen = sizeof(uint16);
					timeout = 0;
					break;
				case 0x8B:		//write
					((uint8 *)msg->response)[0] = if_ble_write_async(msg->context, params->param1, (uint16)params->param2);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x8C:
					((uint8 *)msg->response)[0] = if_ble_try_connect_async(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x8E:
					((uint8 *)msg->response)[0] = if_ble_is_connected_async(msg->context, params->param1);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
				case 0x8F:
					((uint8 *)msg->response)[0] = if_ble_wake_async(msg->context);
					msg->reslen = sizeof(uint8);
					timeout = 0;
					break;
#endif
				default: 			//no API available
					((uint16 *)msg->response)[0] = -1;
					msg->reslen = sizeof(uint16);
					timeout = 0;
					break;
			}
			abort_operation:
			os_dispatch_reply(msg);
		}
		if(timeout > 200) {
			timeout -= 200;
			os_wait(200);
		} else if(timeout > 0) {
			os_wait(timeout);
			timeout = 0;
		} else 
			timeout = 0;
	}
	OS_DEBUG_EXIT();
}
#endif

uint8 g_net_task_stack[14400];
uint8 g_wifi_task_stack[1024];
void if_net_init_config(net_context_p ctx) {
	//net_ssid_p list;
	uint16 len;  
	uint8 i, n1, n2, n3;
	char * ptr;
	//USART_InitTypeDef USART_InitStructure;
	uint8 netbuf[2048];
	os_create_task_static(ctx, if_wifi_task, "wifi", 60, (lp_void)g_wifi_task_stack, sizeof(g_wifi_task_stack));
	len = if_net_command(ctx, NULL, netbuf, 1000);
	//len = if_net_command(ctx, (uint8 *)"AT+RST", netbuf, 200); 							//soft reset
	len = if_net_command(ctx, (uint8 *)"AT+GMR", netbuf, 200); 						//check firmware
#if SHARD_WIFI_MODULE == MOD_ESP32
	len = if_net_command(ctx, (uint8 *)"AT+CWMODE=1", netbuf, 200); 		//mode station
	len = if_net_command(ctx, (uint8 *)"AT+CIPDNS=1, \"8.8.8.8\", \"8.8.4.4\"", netbuf, 200); 		//mode station
	len = if_net_command(ctx, (uint8 *)"AT+CIPDINFO=1", netbuf, 200);		//enable remote_ip info
#if NET_USE_CHANNEL
	len = if_net_command(ctx, (uint8_t *)"AT+CIPMUX=1", netbuf, 200); 					//single channel
#endif
#endif
	

#if SHARD_WIFI_MODULE == MOD_ESP8266
	len = if_net_command(ctx, (uint8 *)"AT+CWMODE_CUR=1", netbuf, 200); 		//mode station
	len = if_net_command(ctx, (uint8 *)"AT+CIPDNS_CUR=1, \"8.8.8.8\", \"8.8.4.4\"", netbuf, 200); 		//mode station
	
#if NET_USE_CHANNEL
	len = if_net_command(ctx, (uint8_t *)"AT+CIPMUX_CUR=1", netbuf, 200); 					//single channel
#endif
#endif
	//sprintf((char *)netbuf, "AT+CIPSTAMAC=\"%02x:%02x:%02x:%02x:%02x:%02x\"",
	//		ctx->mac[0], ctx->mac[1],
	//		ctx->mac[2], ctx->mac[3],
	//		ctx->mac[4], ctx->mac[5]);
	len = if_net_command(ctx, (uint8_t *)"AT+CIPSTAMAC?", netbuf, 200);
	if((ptr = strstr((const char *)netbuf, "+CIPSTAMAC:")) != 0) {
		ptr = ptr + 12;
		for(i=0;i<6;i++) {
			n1 = *ptr++;
			n2 = *ptr++;
			n3 = *ptr++;
			ctx->mac[i] = net_hex2byte(n1) << 4 | net_hex2byte(n2);
		}
		//net_mac2bin((uint8 *)ptr, ctx->mac);
	}
	len = if_net_command(ctx, (uint8_t *)"AT+UART=115200,8,1,0,0", netbuf, 200); 					//check joined access point
	

	//netbuf[len] = 0;
	if(strstr((const char *)netbuf, "OK") != NULL) {
		ctx->baudrate = 115200;			//115200
		ctx->handle.Instance = USART1;
		ctx->handle.Init.BaudRate        	= ctx->baudrate; 
		ctx->handle.Init.WordLength 		= USART_WORDLENGTH_8B; //8位数据
		ctx->handle.Init.StopBits            = USART_STOPBITS_1;	 //停止位1位
		ctx->handle.Init.Parity              	= USART_PARITY_NONE ;	 //无
		ctx->handle.Init.HwFlowCtl 			= UART_HWCONTROL_NONE;
		ctx->handle.Init.OverSampling 		= UART_OVERSAMPLING_16;
		ctx->handle.Init.Mode = USART_MODE_RX | USART_MODE_TX;
		HAL_UART_Init((UART_HandleTypeDef *)ctx);
	}
	len = if_net_command(ctx, (uint8_t *)"AT+CWJAP?", netbuf, 200); 					//check joined access point
	//len = if_net_command(ctx, NULL, netbuf, 2000);
	if(strstr((char *)netbuf, "WIFI CONNECTED") != NULL) { 
		ctx->state |= IF_NET_STATE_CONNECTED;
		//check IP settings
		len = if_net_command(ctx, (uint8_t *)"AT+CIFSR", netbuf, 200); 
	} else {
		//if(if_net_ssid_check(ctx, g_uconfig.ssid_name) == 0) {
		//	if_net_ssid_join(ctx, g_uconfig.ssid_name, g_uconfig.ssid_password);
		//}
	}
	
#if SHARD_RTOS_ENABLED 
	memset(ctx->ipv4, 0, 4);
	//memset(ctx->mac, 0, 6);
	ctx->join = if_net_ssid_join;
	ctx->list = if_net_ssid_list;
	ctx->try_connect = if_net_try_connect;
	//ctx->send_http = if_net_http_send;
	ctx->send_tcp = if_net_tcp_transmit;
	ctx->send_udp = if_net_udp_transmit;
	ctx->exception_handler = NULL;
	ctx->p_ctx = NULL;
	ctx->f_ctx = NULL;
	ctx->prepare_callback = NULL;
	ctx->finish_callback = NULL;
	ctx->cb_depth = 0;
	ctx->fcnctr = 0;
	//net task should have at least 12K stack for wolfSSL
	os_create_task_static(ctx, if_net_task, "net", 127, (lp_void)g_net_task_stack, sizeof(g_net_task_stack));
	//os_create_task(ctx, if_wifi_task, "wifi", 126, 1024);
#else
	//check for saved ssid name
	if_net_ssid_list(ctx);
	memset(ctx->ipv4, 0, 4);
	//memset(ctx->mac, 0, 6);
	//reset static configuration
	memset(ctx->staip, 0, 4);
	memset(ctx->stadns1, 0, 4);
	memset(ctx->stadns2, 0, 4);
	ctx->join = if_net_ssid_join;
	ctx->list = if_net_ssid_list;
	//ctx->send_http = if_net_http_send;
	ctx->send_tcp = if_tcp_transmit;
	ctx->send_udp = if_udp_transmit;
	//if_net_list_network(ctx, &list);
	if_timer_create(if_net_tick, ctx, 5);		//tick every 5 second
#endif
}

void if_net_init(net_context_p ctx) {
	esp8266_init_config(ctx);			//initialize pin and usart1 (ready to transmit data)
}

static uint8 if_net_force_connect_async(net_context_p ctx) {
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
	if(ctx->ssid_list == NULL) {
		len = if_net_command(ctx, (uint8 *)"AT+CWLAP", gba_net_bufstream, 8000); 
		//some delay here (please do something)
		//len = if_net_command(ctx, NULL, gba_net_bufstream, 3000); 
		while(i<len) {
			if(gba_net_bufstream[i] == mark[state]) state++;
			else state = 0;
			if(state == 7) {			//matched
				tlist = if_net_ssid_create(ctx, gba_net_bufstream + i + 1);
				if(tlist == NULL) break;		//unable to create ssid
				if(clist == NULL) clist = tlist;
				else plist->next = tlist;
				plist = tlist;
				
				while(gba_net_bufstream[i++] != ',');
				while(gba_net_bufstream[i++] != ',');
				j=0;
				while((cbuf[j++] = gba_net_bufstream[i++]) != ',');
				tlist->att = atoi((const char *)cbuf);
				num_ssid++;
			}
			i++;
		}
		if(clist != NULL) {
			//check if ssid_list is not empty
			if(ctx->ssid_list != NULL) if_net_ssid_clear(ctx->ssid_list);
			//set current context ssid list
			ctx->ssid_list = clist;
		}
	}
	//try connecting to any available SSID
	if(ctx->ssid_list != NULL) {
		//try connecting to stored ssidrec
		for(i=0;i<4;i++) {
			if(if_net_ssidrec_read(ctx, i, &rec) != 0) continue;		//invalid record
			tlist = ctx->ssid_list;
			while(tlist != NULL) {
				if(strncmp((const char *)rec.name, (const char *)tlist->name, 30) == 0) {
					//try joining
					//if(w_idx == try_idx) {
					try_idx++;
					ret = if_net_ssid_join_async(ctx, rec.name, rec.username, rec.password);
					//}
					w_idx ++;
					goto exit_try_connect;
				}
				tlist = tlist->next;
			}
		}
		try_idx = 0;		//no ssid matched with current records
	}
	exit_try_connect:
	return ret;
}

uint8 if_net_try_connect_async(net_context_p ctx) {
	uint8 buffer[128];
	uint8 ret = -1;
	if(!NET_BUSY(ctx)) {
		NET_LOCK(ctx);
		if(if_net_is_connected(ctx, gba_net_bufstream) == 0)  {
			ret = 0;
		} else {
			//try connect
			if(if_net_force_connect_async(ctx) == 0) {
				//conected successfully
				ret = 0;
			} else {
				//use static IP
				if(memcmp(ctx->staip, "\x0\x0\x0\x0", 4) == 0) goto exit_try_connect;
				if(memcmp(ctx->staip, "\xff\xff\xff\xff", 4) == 0) goto exit_try_connect;
				sprintf((char *)buffer, "AT+CIPAP_CUR=\"%d.%d.%d.%d\"", ctx->staip[0], ctx->staip[1], ctx->staip[2], ctx->staip[3]);
				if_net_command(ctx, buffer, gba_net_bufstream, 250); 
				if(strstr((const char *)gba_net_bufstream, "OK") != NULL) {
					if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->exception_handler != NULL) 
						ctx->exception_handler(ctx, "use static IP");
					//force connect with static IP
					ret = if_net_force_connect_async(ctx);
				}
			}
		}
		exit_try_connect:
		NET_UNLOCK(ctx);
	}
	return ret;
}

uint8 if_net_ssidrec_count(void * handle) {
	OS_DEBUG_ENTRY(if_net_ssidrec_count);
	uint8 i = 0;
	uint8 counter = 0;
	net_ssidrec temp;
	for(i=0;i<4;i++) {
		if(if_net_ssidrec_read(NULL, i, &temp) == 0) { counter++; }
	}
	OS_DEBUG_EXIT();
	return counter;
}

uint8 if_net_ssidrec_read(void * handle, uint8 index, net_ssidrec * record) {
	OS_DEBUG_ENTRY(if_net_ssidrec_read);
	uint8 stat = 0;
	index = index % 4;
	if_flash_data_read(NULL, SHARD_SSIDREC_START + (index * sizeof(net_ssidrec)), (uint8 *)record, sizeof(net_ssidrec));
	if(record->magic != NET_SSIDREC_MAGIC) stat = -1;
	OS_DEBUG_EXIT();
	return stat;
}

void if_net_ssidrec_push(void * handle, uint8 * name, username, uint8 * password) {
	OS_DEBUG_ENTRY(if_net_ssidrec_push);
	net_ssidrec rec[4];
	net_ssidrec temp;
	uint8 i,j;
	uint8 cur_idx = 0;
	//check ssidname and password length, if overflow, do not store record
	if(strlen((const char *)name) >= 29) return;
	if(strlen((const char *)username) >= 31) return;
	if(strlen((const char *)password) >= 31) return;
	//read any previous records
	if_flash_data_read(NULL, SHARD_SSIDREC_START, (uint8 *)&rec, sizeof(rec));
	//increment counter in-case oid already existed
	for(i=0;i<4;i++) {
		if(strncmp((const char *)rec[i].name, (const char *)name, 30) == 0) {
			rec[i].ordinal++;
			memcpy(rec[cur_idx].username, username, 32);
			memcpy(rec[cur_idx].password, password, 32);
			break;
		}
	}
	if(i == 4) i = 3;		//put on last record
	cur_idx = i;
	//create new record
	rec[cur_idx].magic = NET_SSIDREC_MAGIC;
	rec[cur_idx].ordinal = 1;
	memcpy(rec[cur_idx].name, name, 30);
	memcpy(rec[cur_idx].username, username, 32);
	memcpy(rec[cur_idx].password, password, 32);
	//sort record by ordinal
	for(j=0;j<3;j++) {
		if(rec[j].ordinal < rec[j+1].ordinal) {
			memcpy(&temp, &rec[j+1], sizeof(net_ssidrec));
			memcpy(&rec[j+1], &rec[j], sizeof(net_ssidrec));
			memcpy(&rec[j], &temp, sizeof(net_ssidrec));
		}			
	}
	if_flash_data_write(NULL, SHARD_SSIDREC_START, (uint8 *)&rec, sizeof(rec));
	OS_DEBUG_EXIT();
}

//callback APIs
void if_net_set_prepare_context(net_context_p ctx, void * cb_ctx) { ctx->p_ctx = cb_ctx; }
void if_net_set_prepare_callback(net_context_p ctx, void (* prepare)(void *)) { ctx->prepare_callback = prepare; }
void if_net_set_finish_context(net_context_p ctx, void * cb_ctx) { ctx->f_ctx = cb_ctx; }
void if_net_set_finish_callback(net_context_p ctx, void (* finish)(void *)) { ctx->finish_callback = finish; }
void if_net_set_exception_callback(net_context_p ctx, void  (* exception)(net_context_p, const char * )) { ctx->exception_handler = exception; }


#endif 		//END OF ESP8266 MODULE

#if SHARD_BLE_MODULE == MOD_ESP32
static uint16 if_ble_command(bt_context_p ctx, uint8_t * command, uint8 * response, uint32 timeout) {
	uint16 len = 0;
	len = if_net_command(ctx->handle, command, response, 40);
	len += if_net_data(ctx->handle, NULL, 0, response + len, timeout);
	return len;
}

uint16 net_hex2bin(uint8 * hexstring, uint8 * bytes, uint8 length) {
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

void net_mac2bin(uint8* mac_str, uint8 * bin) {
	uint8 c;
	uint8 i;
	for(i=0;i<6;i++) {
		c = net_hex2byte(mac_str[i * 3]);
		c <<= 4;
		c |= net_hex2byte(mac_str[(i * 3) + 1]);
		bin[i] = c;
	}
}

uint8 if_ble_get_mac(bt_context_p ctx) {
	uint8 buffer[256];
	uint8 len;
	char * ptr;
	uint8 ret = -1;
	memset(buffer, 0, sizeof(buffer));
	len = if_ble_command(ctx, (uint8 *)"AT+BLEADDR?", buffer, 200);
	if((ptr = strstr((char *)buffer, "+BLEADDR:")) != NULL) {
		net_mac2bin((uint8 *)ptr + 9, ctx->host_mac);
		ret = 0;
	}
	return ret;
}

static bt_device_p if_bt_device_create(bt_context_p ctx, uint8 * mac) {
	uint8 i = 0;
	uint8 dev_macs[256];
	uint8 state = 0;
	uint8 parsing = 1;
	uint8 bufidx = 0;
	bt_device_p dev = (bt_device_p)malloc(sizeof(bt_device));
	memset(dev_macs, 0, sizeof(dev_macs));
	if(dev == NULL) return NULL;
	memcpy(dev_macs, mac, 17);
	net_mac2bin(dev_macs, dev->mac);
	sprintf((char *)dev->name, "%02x:%02x:%02x:%02x:%02x:%02x", dev->mac[0], dev->mac[1], dev->mac[2], dev->mac[3], dev->mac[4], dev->mac[5]);
	dev->next = NULL;
	dev->ctx = ctx;
	dev->srv_id = 0;
	dev->chr_id = 0;
	return dev;
}

void if_bt_device_clear(bt_device_p root) {
	bt_device_p tlist = root;
	bt_device_p plist;
	while(tlist != NULL) {
		plist = tlist;
		tlist = tlist->next;
		memset(plist, 0, sizeof(bt_device));
		free(plist);
	}
}

bt_device_p if_ble_dev_check(bt_context * ctx, uint8 * mac) {
	uint8 dev_count = 0;
	bt_device_p iterator = NULL;
	iterator = ctx->dev_list;
	while(iterator != NULL) {
		if(memcmp(iterator->mac, mac, 6) == 0) return iterator;
		iterator = iterator->next;
	}
	return iterator;
}

void if_ble_btrec_push(void * handle, uint8 * mac, uint8 * password) {
	OS_DEBUG_ENTRY(if_ble_btrec_push);
	net_btrec rec;
	uint8 cur_idx = 0;
	memset(&rec, 0, sizeof(rec));
	//read any previous records
	if_flash_data_read(NULL, SHARD_BTREC_START, (uint8 *)&rec, sizeof(rec));
	rec.magic = 0xEF;
	memcpy(rec.mac, mac, 6);
	if(password != NULL) memcpy(rec.password, password, 32);
	if_flash_data_write(NULL, SHARD_BTREC_START, (uint8 *)&rec, sizeof(rec));
	OS_DEBUG_EXIT();
}

uint8 if_ble_btrec_read(void * handle, net_btrec * rec) {
	if_flash_data_read(NULL, SHARD_BTREC_START, (uint8 *)rec, sizeof(net_btrec));
	if(rec->magic != 0xEF) return -1;
	return 0;
}

uint8 if_ble_dev_list_async(bt_context * ctx) {
	OS_DEBUG_ENTRY(if_ble_dev_list_async);
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
	len = if_ble_command(ctx, (uint8 *)"AT+BLESCAN=1,1", buffer, 400);		//scan for 2 seconds
	if(len != 0) { if_bt_device_clear(ctx->dev_list); ctx->dev_list = NULL; }
	iterator = ctx->dev_list;
	ptr = buffer;
	buffer[len] = 0;
	while((ptr = (uint8 *)strstr((char *)ptr, "+BLESCAN:")) != NULL) {
		if(iterator == NULL) {
			iterator = if_bt_device_create(ctx, ptr + 9);		//mac address
			ctx->dev_list = iterator;
			dev_count ++;
		} else {
			net_mac2bin(ptr + 9, mac);
			if(if_ble_dev_check(ctx, mac) == NULL) {			//check if device already in the list
				iterator->next = if_bt_device_create(ctx, ptr + 9);		//mac address
				dev_count ++;
			}
		}
		ptr++;		//advanced pointer to next character
	}
	exit_dev_list:
	OS_DEBUG_EXIT();
	return dev_count;
}

uint8 if_ble_try_connect_async(bt_context * ctx) {
	OS_DEBUG_ENTRY(if_ble_try_connect_async);
	net_btrec rec;
	bt_device_p dev;
	uint8 ret = -1;
	if((ctx->state & BLE_STATE_INITIALIZED) == 0) goto exit_try_connect;
	if((ret = if_ble_is_connected_async(ctx, ctx->slave_mac)) == 0) goto exit_try_connect;		//check if already connected 
	if(if_ble_btrec_read(NULL, &rec) == 0) {		//read for record and validate, if no record avail -> return
		if(if_ble_dev_list_async(ctx) != 0) {			//try scanning for any bluetooth devices
			dev = ctx->dev_list;
			while(dev != NULL) {
				if(memcmp(dev->mac, rec.mac, 6) == 0) {		//check for matching mac address
					ret = if_ble_connect_async(ctx, dev);
					break;
				}
				dev = dev->next;
			}
		}
	}
	exit_try_connect:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 if_ble_is_connected_async(bt_context_p ctx, uint8 * mac) {
	OS_DEBUG_ENTRY(if_ble_is_connected_async);
	uint8 buffer[2048];
	uint8 len;
	uint8 ret = -1;
	uint8 hex_buffer[32];
	char * ptr;
	if(ctx->state & BLE_STATE_INITIALIZED) {
		len = if_ble_command(ctx, (uint8 *)"AT+BLEINIT=1", buffer, 200);
		buffer[len] = 0;
		len = if_ble_command(ctx, (uint8 *)"AT+BLECONN?", buffer, 400);
		buffer[len] = 0;
		if((ptr = strstr((char *)buffer, "+BLECONN:0")) != NULL) {
			ctx->state |= BLE_STATE_CONNECTED;
			if(mac != NULL) {
				net_mac2bin((uint8 *)ptr + 11, mac);
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

uint8 if_ble_connect_async(bt_context * ctx, bt_device_p dev) {
	OS_DEBUG_ENTRY(if_ble_connect_async);
	uint8 buffer[2048];
	uint8 len;
	uint8 ret = -1;
	uint8 hex_buffer[32];
	char * ptr;
	sprintf((char *)hex_buffer, "%02x:%02x:%02x:%02x:%02x:%02x", dev->mac[0], dev->mac[1], dev->mac[2], dev->mac[3], dev->mac[4], dev->mac[5]);
	sprintf((char *)buffer, "AT+BLECONN=0,\"%s\"", hex_buffer);		//use connection index=0
	if(ctx->state & BLE_STATE_CONNECTED) if_ble_disconnect_async(ctx);
	//if(ctx->state & IF_BLE_CONNECTED) goto exit_connect;	//already connected
	if(ctx->state & BLE_STATE_INITIALIZED) {
		if((ctx->handle->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->handle->exception_handler != NULL) 
			ctx->handle->exception_handler(ctx->handle, "bluetooth connecting");
		len = if_ble_command(ctx, buffer, buffer, 400);
		if((ptr = strstr((char *)buffer, "+BLECONN:")) != NULL) {
			memcpy(ctx->slave_mac, dev->mac, 6);
			if((ctx->handle->state & IF_NET_STATE_CRITICAL_SECTION) == 0 && ctx->handle->exception_handler != NULL) 
				ctx->handle->exception_handler(ctx->handle, "enumerating services");
			ctx->state |= BLE_STATE_CONNECTED;
			ctx->services = if_ble_list_services(ctx);
			if_ble_btrec_push(NULL, dev->mac, NULL);
			ret = 0;
		}
	}
	exit_connect:
	OS_DEBUG_EXIT();
	return ret;
}

uint8 if_ble_disconnect_async(bt_context * ctx) {
	uint8 buffer[2048];
	uint8 len = 0;
	uint8 ret = -1;
	if((ctx->state & BLE_STATE_CONNECTED) == 0) return ret;
	len = if_ble_command(ctx, (uint8 *)"AT+BLEDISCONN=0", buffer, 200);
	if(strstr((char *)buffer, "OK") != NULL) { 
		ctx->state &= ~(BLE_STATE_CONNECTED | BLE_STATE_OPENED);		//close and disconnect
		if(ctx->services != NULL) os_free(ctx->services);
		ctx->services = NULL;
		ret = 0;
	}
	return ret;
}

uint8 if_ble_dev_list(bt_context * ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x80;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_connect(bt_context * ctx, bt_device_p dev) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x81;
	req.param1 = dev;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_is_connected(bt_context_p ctx, uint8 * mac) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x8E;
	req.param1 = mac;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_disconnect(bt_context * ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x82;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}


uint8 if_ble_try_connect(bt_context * ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x8C;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_wake(bt_context * ctx) {
	//struct net_async_params req;
	//os_message * msg;
	uint8 ret = 0;
	//req.type = 0x8F;
	//msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	//os_delete_message(msg);
	return ret;
}

uint8 if_ble_wake_async(bt_context * ctx) {
	uint16 len;
	uint8 ret = -1;
	uint8 buffer[64];
	if((ctx->state & BLE_STATE_INITIALIZED) == 0) return ret;
	len = if_ble_command(ctx, (uint8 *)"AT+BLEINIT=1", buffer, 200);
	if(strstr((char *)buffer, "OK") != NULL) {
		ctx->state |= BLE_STATE_INITIALIZED;
		ret = 0;
	}
	return ret;
}

uint8 if_ble_sleep(bt_context * ctx) {
	//if_ble_disconnect(ctx);
	//if_bt_device_clear(ctx->dev_list); 
	//ctx->dev_list = NULL; 
	return 0;
}

uint8 if_ble_init(bt_context * ctx, void * handle) {
	uint16 len;
	uint8 buffer[64];
	if(handle == NULL) return -1;
	memset(ctx, 0, sizeof(bt_context));
	ctx->handle = handle;
	len = if_ble_command(ctx, (uint8 *)"AT+BLEINIT=1", buffer, 1000);
	if(strstr((char *)buffer, "OK") != NULL) {
		ctx->state |= BLE_STATE_INITIALIZED;
		return 0;
	}
	return -1;
}

uint8 if_ble_write(bt_handle_p handle, uint8 * buffer, uint16 size) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x8B;
	req.param1 = buffer;
	req.param2 = (void *)(uint32)size;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(handle, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

void if_btdev_init(bt_context * ctx) {
	
}

bt_service_p if_ble_list_services(bt_context_p ctx) {
	OS_DEBUG_ENTRY(if_ble_list_services);
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
	len = if_ble_command(ctx, (uint8 *)"AT+BLEGATTCPRIMSRV=0", gba_net_bufstream, 400);
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
			net_hex2bin(ptr + 23, iterator->srv_uuid, 4);
			iterator->srv_uuid_len = 2;
			sindex += (sizeof(bt_service) + 2);
		} else {
			net_hex2bin(ptr + 23, iterator->srv_uuid, 32);
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
		len = if_ble_command(ctx, (uint8 *)buffer, gba_net_bufstream, 200);
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
				net_hex2bin(ptr + 29, chr->chr_uuid , 4);
				chr->attr = (net_hex2byte(ptr[36]) << 4) | net_hex2byte(ptr[37]);		//set attribute for current characteristic
				chr->chr_uuid_len = 2;
			} else {
				net_hex2bin(ptr + 31, chr->chr_uuid , 4);
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

uint8 if_ble_open_static(bt_context * ctx, bt_chr_p chr, bt_handle_p handle) {
	if(chr == NULL) return -1;
	if(handle != NULL) {
		handle->ctx = ctx;
		handle->chr = chr;
	}
	return 0;
}	

bt_handle_p if_ble_open(bt_context * ctx, bt_chr_p chr) {
	OS_DEBUG_ENTRY(if_ble_open);
	bt_handle_p handle = NULL;
	if(chr == NULL) goto exit_ble_open;
	//if(chr->state & 0x01) return handle;		//check if already locked
	handle = os_alloc(sizeof(bt_handle));
	if(handle != NULL) {
		handle->ctx = ctx;
		handle->chr = chr;
		//chr->state |= 0x01;		//lock state
	}
	exit_ble_open:
	OS_DEBUG_EXIT();
	return handle;
}

bt_service * if_ble_find_service(bt_context * ctx, uint8 * uuid, uint8 uuid_len) {
	bt_service * iterator = NULL;
	if(ctx == NULL) return 0;
	if((ctx->state & BLE_STATE_CONNECTED) == 0) return 0;			//no device connected
	iterator = ctx->services;
	while(iterator != NULL) {
		if(iterator->srv_uuid_len == uuid_len) {
			if(memcmp(iterator->srv_uuid, uuid, uuid_len) == 0) return iterator;
		}
		iterator = iterator->next;
	}
	return iterator;
}

bt_chr * if_ble_find_char(bt_service * srv, uint8 * uuid, uint8 uuid_len) {
	bt_chr * iterator = NULL;
	if(srv == NULL) return 0;
	iterator = srv->chrs;
	while(iterator != NULL) {
		if(iterator->chr_uuid_len == uuid_len) {
			if(memcmp(iterator->chr_uuid, uuid, uuid_len) == 0) return iterator;
		}
		iterator = iterator->next;
	}
	return iterator;
}


uint16 if_ble_read(bt_handle_p handle, uint8 * buffer, uint16 size) {
	struct net_async_params req;
	os_message * msg;
	uint8 ret;
	req.type = 0x8A;
	req.param1 = buffer;
	req.param2 = (void *)(uint32)size;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(handle, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint8 if_ble_write_async(bt_handle_p handle, uint8 * data, uint16 size) {
	OS_DEBUG_ENTRY(if_ble_write_async);
	uint16 len;
	uint8 ret = -1;
	uint8 buffer[1500];
	uint8 chid = handle->chr->chr_id;
	uint8 svid = handle->chr->service->srv_id;
	if(handle == NULL) goto exit_ble_write;
	sprintf((char *)buffer, "AT+BLEGATTCWR=0,%d,%d,,%d", svid, chid, size);
	//if_ble_command(handle->ctx, buffer, buffer, timeout);
	len = if_net_command(handle->ctx->handle, buffer, buffer, 200);
	buffer[len] = 0;
	if(strstr((const char *)buffer, ">") != NULL) {
		if_net_data(handle->ctx->handle, data, size, NULL, 500);
		ret = 0;
	}
	exit_ble_write:
	OS_DEBUG_EXIT();
	return ret;
}

uint16 if_ble_read_async(bt_handle_p handle, uint8 * response, uint16 size) {
	OS_DEBUG_ENTRY(if_ble_read_async);
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
	len = if_ble_command(handle->ctx, buffer, buffer, 200);
	//start read characteristics
	sprintf((char *)buffer, "AT+BLEGATTCRD=0,%d,%d", svid, chid);
	//len = if_net_command(handle->ctx->handle, buffer, buffer, 200);
	len = if_ble_command(handle->ctx, buffer, response, 400);
	if((ptr = (uint8 *)strstr((const char *)response, "+BLEGATTCRD:")) != NULL) {
		for(i=14;i<18;i++) { if(ptr[i] == ',') { response[i] = 0; break; } }
		ret = atoi((const char *)ptr + 14);
		memcpy(response, ptr + (i + 1), ret);
	}
	exit_ble_read:
	OS_DEBUG_EXIT();
	return ret;
}

void if_ble_close(bt_handle_p handle) {
	OS_DEBUG_ENTRY(if_ble_close);
	if(handle != NULL) {
		handle->chr->state = 0;		//unlock handle
		os_free(handle);
	}
	OS_DEBUG_EXIT();
}

#endif