#include "defs.h"
#include "config.h"
#include "..\inc\if_apis.h"
#include <string.h>
#include "if_net.h"
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#include "..\..\core\inc\os_msg.h"
#endif

#if SHARD_WIFI_MODULE == MOD_RTL8710
//RTL8710		Realtek Driver
uint8 gba_net_buffer[NET_BUFFER_SIZE];
uint8 gba_recv_buffer[256];
uint8 gba_net_bufhead = 0;
uint8 gba_net_buftail = 0;
uint8 gb_net_bufrdy = 0;
static net_context_p g_current_context = NULL;

#define RTL8710_TRANSPARENT_TRANSMISSION			1

void USART1_IRQHandler(void)
{
	uint8 c;
	if(g_current_context == NULL) return;
	if((USART1->SR &USART_IT_PE) == USART_IT_PE)
	{ 
		USART1->SR &= ~USART_FLAG_PE;
		return;
	}
	if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)g_current_context, USART_FLAG_RXNE) != RESET)
	//if((USART1->SR & USART_IT_RXNE) == USART_IT_RXNE)
	{ 	
	    //gba_net_buffer[gba_net_bufhead++] = USART1->DR;
		gba_recv_buffer[gba_net_bufhead++] = USART1->DR;
		gb_net_bufrdy = 1;
	} 
}

static void if_net_reset(net_context_p ctx) {
	HAL_GPIO_WritePin(ctx->port, ctx->rst,GPIO_PIN_SET);
	//HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
	//GPIO_ResetBits(ctx->port, ctx->rst);
	if_delay(160);
	HAL_GPIO_WritePin(ctx->port, ctx->rst, GPIO_PIN_RESET);
	//GPIO_SetBits(ctx->port, ctx->rst);
	if_delay(160);
	HAL_GPIO_WritePin(ctx->port, ctx->rst, GPIO_PIN_SET);
	//GPIO_ResetBits(ctx->port, ctx->rst);
	if_delay(2000);
}

void rtl8710_init_config(net_context_p ctx, uint32 baudrate) {
	GPIO_InitTypeDef GPIO_InitStructure;             
	//USART_InitTypeDef USART_InitStructure;
	//UART_HandleTypeDef UART_InitStructure;
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	// 使能端口时钟，重要！！！  
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);	// 使能端口时钟，重要！！！ 
	//GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	//GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();
	//icc clock
   	/* USART1 Tx (PA9) */
  	GPIO_InitStructure.Pin = GPIO_PIN_9;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;	 					//default AF_PP
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;								//default NOPULL
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
	GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	//GPIO_PinAFConfig(GPIOA, GPIO_PIN_9, GPIO_AF_USART1);
  	/* USART1 Rx (PA10)  */
  	GPIO_InitStructure.Pin = GPIO_PIN_10;
  	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;						//default AF_PP
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;								//default PULLUP
  	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;		//default HIGH
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
	ctx->rst = GPIO_PIN_8;
	ctx->cs = GPIO_PIN_11;
	ctx->port = GPIOA;
	ctx->baudrate = baudrate;
	ctx->handle.Instance = USART1;
	ctx->handle.Init.BaudRate        	= ctx->baudrate; 
	ctx->handle.Init.WordLength 		= USART_WORDLENGTH_8B; 	//8位数据
	ctx->handle.Init.StopBits            = USART_STOPBITS_1;	 		//停止位1位
	ctx->handle.Init.Parity              	= USART_PARITY_NONE ;	 	//无	
	ctx->handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	ctx->handle.Init.OverSampling = UART_OVERSAMPLING_16;	//UART_OVERSAMPLING_16;
	//USART_InitStructure.HardwareFlowControl = USART_HardwareFlowControl_None;
	ctx->handle.Init.Mode = USART_MODE_TX_RX;
	HAL_UART_Init((UART_HandleTypeDef *)ctx);
	//USART_ITConfig(ctx->handle, USART_IT_RXNE, ENABLE);
	//USART_Cmd(ctx->handle, ENABLE);
	GPIO_SetBits(GPIOB, GPIO_PIN_1);				//enable device
	//if_net_reset(ctx);					//reset device
}

static void if_net_sendbyte(net_context_p ctx, uint8 b) {
   	ctx->handle.Instance->DR = b;
   	while (!(ctx->handle.Instance->SR & USART_FLAG_TXE));
   	//while (!(ctx->handle->SR & USART_FLAG_TC)); 
	//if_delay(1);
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

static uint16 if_net_command(net_context_p ctx, uint8_t * command, uint8 * response, uint32 timeout) {
	uint32 i;
	uint8 c;
	uint16 len =0;
	uint16 state = 0;
	//uint8 cbuffer[256];
	uint16 slen;	// = strlen((const char *)cbuffer);
	gb_net_bufrdy = 0;
	timeout *= 100000;
	//__HAL_RCC_USART1_CLK_ENABLE();
	if(command != NULL) {
		if_net_sendstring(ctx, command);
		if_net_sendstring(ctx, (uint8 *)"\r\n");
		gb_net_bufrdy = 0;
	}
#if SHARD_RTOS_ENABLED
	//for(i=0;i<timeout;i++) { if(gb_net_bufrdy != 0) break; }
	//for(i=0;i<500 & gb_net_bufrdy != 0;i++) os_wait(2);
#else
	for(i=0;i<timeout;i++) { if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) break; }
	if(i == timeout) goto exit_send_command;
#endif
#if SHARD_RTOS_ENABLED
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TC);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_PE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_ERR);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_IDLE);
	__HAL_USART_DISABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_TXE);
	__HAL_USART_ENABLE_IT((UART_HandleTypeDef *)ctx, USART_IT_RXNE);
	__HAL_UART_CLEAR_PEFLAG((UART_HandleTypeDef *)ctx);
	g_current_context = ctx;
	gba_net_buftail = gba_net_bufhead = 0;
	if(ctx->state & IF_NET_STATE_CRITICAL_SECTION) {
		//for(i=0;i<4000000 && gb_net_bufrdy == 0;i++);		//x seconds waiting till response
		//while(gb_net_bufrdy == 0);
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) == RESET);
		for(i=0;i<60000;i++) {  
			if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
				response[len++] = ctx->handle.Instance->DR;
				i = 0;
			}
		}
	} else {
		NVIC_EnableIRQ(USART1_IRQn);
		for(i=0;i<8000 && gb_net_bufrdy == 0;i++) os_wait(1);		//8 seconds waiting till response
		//while(gb_net_bufrdy == 0);
		for(i=0;i<100000;i++) {  
			while(gba_net_buftail != gba_net_bufhead) {
				response[len++] = gba_recv_buffer[gba_net_buftail++];
				i=0;
			}
		}
		NVIC_DisableIRQ(USART1_IRQn);
	}
#else
	for(i=0;i<20000;i++) {  
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
			response[len++] = ctx->handle.Instance->DR;
			i = 0;
		}
	}
#endif
	exit_send_command:
	//__HAL_RCC_USART1_CLK_DISABLE();
	return len;
}

static uint16 if_net_data(net_context_p ctx, uint8_t * data, uint16 length, uint8 * response, uint32 timeout) {
	uint32 i;
	uint8 c;
	uint16 len =0;
	uint16 state = 0;
	//uint8 cbuffer[256];
	uint16 slen;	// = strlen((const char *)cbuffer);
	gb_net_bufrdy = 0;
	timeout *= 10000;
	//__HAL_RCC_USART1_CLK_ENABLE();
	if(data != NULL) {
		for(i=0;i<length;i++) if_net_sendbyte(ctx, data[i]);
	}
	for(i=0;i<timeout;i++) { if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) break; }
	if(i == timeout) goto exit_send_data;
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
		while(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) == RESET);
		for(i=0;i<60000;i++) {  
			if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
				response[len++] = ctx->handle.Instance->DR;
				i = 0;
			}
		}
	} else {
		NVIC_EnableIRQ(USART1_IRQn);
		for(i=0;i<60000;i++) {  
			while(gba_net_buftail != gba_net_bufhead) {
				response[len++] = gba_recv_buffer[gba_net_buftail++];
				i=0;
			}
		}
		NVIC_DisableIRQ(USART1_IRQn);
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
			case '\n':
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
						//channel
						
						break;
					case 3:
						//encryption level
						//ssid->sec = atoi((const char *)cbuf);
						ssid->sec = 1;
						break;
					case 4:
						//attenuation
						ssid->att = atoi((const char *)cbuf);
						break;
					case 5: 
						//mac address
						break;
				}
				state ++;
				bufidx = 0;
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

static uint8 if_net_get_ipconfig(net_context_p ctx) {
	uint8 cbuf[256];
	uint16 len;
	uint8 * ptr;
	uint8 * sec_buf;
	uint8 * ssid;
	uint8 i, j;
	//get IP settings
	len = if_net_command(ctx, (uint8_t *)"ATW?", cbuf, 200); 			//check wifi configuration
	cbuf[len] = 0;		//eos
	//check IP settings here (STAIP,"0.0.0.0")
	memset(ctx->ipv4, 0, 4);
	//if((ptr = (uint8 *)strstr((char *)cbuf, "+CIFSR:STAIP,")) != NULL) {
	ptr = cbuf;
	//if((uint8 *)strstr((char *)cbuf, "[ATW?]") != NULL) {
	while(*ptr++ != ',');		//skip mode
	ssid = ptr;
	while(*ptr++ != ',');		//skip ssid
	if((ssid + 1) != ptr) {		//check if ssid=empty
		while(*ptr++ != ',');		//skip channel
		sec_buf = ptr;
		while(*ptr++ != ',') ;
		if(memcmp(sec_buf, "WEP", 3) == 0) while(*ptr++ != ',') ;		//skip key_id
		
		while(*ptr++ != ',') ;	//skip pwd
		while(*ptr++ != ',') ;	//skip mac
		//ip address
		for(i=0;i<4;i++) {
			for(j=0;j<4;j++) {
				cbuf[j] = *ptr++;
				switch(cbuf[j]) {
					case ',':
					case '.':
						cbuf[j] = 0;		//eos
						ctx->ipv4[i] = atoi((const char *)cbuf);
						j=4;
						break;
					default: break;
				}
			}
		}
	}
	return memcmp(ctx->ipv4, "\x0\x0\x0\x0", 4) != 0;
}

static uint8 if_net_is_connected(net_context_p ctx, uint8 * ssidname) {
	uint16 len;
	int16 att;
	uint8 * ptr;
	uint8 i=0,j,k;
	uint ret = -1;
	uint8 cbuf[8];
	uint8 netbuf[2048];
	ctx->state &= ~IF_NET_STATE_CONNECTED;
	if_net_wake(ctx);
	if(if_net_get_ipconfig(ctx)) {
		ctx->state |= IF_NET_STATE_CONNECTED;
		ret = 0;
	}
	if_net_sleep(ctx);
	return ret;
}

uint8 if_net_ssid_list(net_context_p ctx) {
	uint16 len;
	uint16 i = 0,j;
	uint8 num_ssid = 0;
	uint8 cbuf[8];
	uint8 state = 0;
	uint8 e_state = 0;
	uint8 mark[] = "AP : ";
	uint8 end_mark[] = "[ATWS]";
	net_ssid_p clist = NULL;
	net_ssid_p plist = NULL;
	net_ssid_p tlist;
	uint8 ssidname[256];
	NET_LOCK(ctx);
	if_net_wake(ctx);
	len = if_net_command(ctx, (uint8 *)"ATWS", gba_net_buffer, 100); 
	//some delay here (please do something)
	len = if_net_command(ctx, NULL, gba_net_buffer, 2000); 
#if 1
	while(i<len) {
		//check for AP :
		if(gba_net_buffer[i] == mark[state]) state++;
		else state = 0;
		//check for end of command
		if(gba_net_buffer[i] == end_mark[state]) e_state++;
		else e_state = 0;
		//check for AP info
		if(state == 5) {			//matched
			tlist = if_net_ssid_create(ctx, gba_net_buffer + i + 1);
			if(tlist == NULL) break;		//unable to create ssid
			if(clist == NULL) clist = tlist;
			else plist->next = tlist;
			plist = tlist;
			
			while(gba_net_buffer[i++] != ',');	//ssid name
			while(gba_net_buffer[i++] != ',');	//channel
			while(gba_net_buffer[i++] != ',');	//security mode
			while(gba_net_buffer[i++] != ',');	//attenuation
			j=0;
			while((cbuf[j++] = gba_net_buffer[i++]) != ',');
			tlist->att = atoi((const char *)cbuf);
			num_ssid++;
		}
		//check of end of command
		if(e_state == 6) break;
		i++;
	}
	//check if ssid_list is not empty
	if(ctx->ssid_list != NULL) if_net_ssid_clear(ctx->ssid_list);
	//set current context ssid list
	ctx->ssid_list = clist;
	//check connected ssid and assigned ip address
	if(clist != NULL && if_net_is_connected(ctx, ssidname) == 0) {
		plist = if_net_ssid_check(ctx, ssidname);
		plist->state |= IF_NET_STATE_CONNECTED;
		if(if_net_get_ipconfig(ctx)) plist->state |= IF_NET_STATE_IP_ASSIGNED;
	}
#endif
	if_net_sleep(ctx);
	NET_UNLOCK(ctx);
	return num_ssid;
}

uint8 if_net_ssid_join(net_context_p ctx, uint8 * ssidname, uint8 * username, uint8 * password) {
	char cbuf[128];
	uint8 ret = 1;			//unable to connect (wrong password)
	uint16 len;
	uint8 * ptr;
	uint8 i, j;
	uint8 netbuf[128];
	//AT+CWDHCP=<mode>,<en> 
	NET_LOCK(ctx);
	if_net_wake(ctx); 
	if_net_command(ctx, (uint8_t *)"ATWD", netbuf, 200); 		//disconnect AP
	sprintf(cbuf, "ATPN=\"%s\",%s", ssidname, password);		//connect AP
	len = if_net_command(ctx, (uint8_t *)cbuf, netbuf, 200); 
	netbuf[len] = 0;
	//some delay here (please do something)
	ctx->state &= ~IF_NET_STATE_CONNECTED;
	//while(1) {
	if(strstr((char *)netbuf, "[ATPN] OK") != NULL) ctx->state |= IF_NET_STATE_CONNECTED | IF_NET_STATE_IP_ASSIGNED;
	//if(strstr((char *)netbuf, "WIFI GOT IP") != NULL) ctx->state |= IF_NET_STATE_IP_ASSIGNED;
	//if(strstr((char *)netbuf, "FAIL") != NULL) break;
	//if(strstr((char *)netbuf, "OK") != NULL) break;
	if(ctx->state & IF_NET_STATE_CONNECTED) {
		if_net_command(ctx, (uint8_t *)"ATPG=1", netbuf, 200); 			//set auto-connect
		//get ip configuration
		if_net_get_ipconfig(ctx);														//get ip configuration
		if(memcmp(ctx->ipv4, "\x0\x0\x0\x0", 4) == 0) {
			ret = 2;		//no IP assigned
		} else { 
			ret = 0;		//no error
		}
	}
	if_net_sleep(ctx);
	NET_UNLOCK(ctx);
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

uint16 if_net_tcp_connect(net_context_p ctx, uint8 * host, uint16 port) {
	uint16 len;
	uint8 cbuf[90];
	uint8 ip4ddr[20];
	uint8 rbuf[90];
	uint16 con_id;
	uint8 * ptr;
	uint8 t = 5;
	uint8 retry = 3;
	//if_net_command(ctx, (uint8 *)"AT+CIPMUX=0", cbuf, 200); 				//single connection
	//if_net_tcp_close(ctx);																	//close existing connection (if exist)
	goto use_hardware_dns;
	if(net_is_ip4ddr(host) != 0) {
		if(dns_translate(ctx, host, ip4ddr) != 0) goto use_hardware_dns;			//try translate domain name address to ip address
		//if(net_is_ip4ddr(ip4ddr) != 0) return -1;									//dns failed
		sprintf((char *)cbuf, "ATPC=0,%s,%d", ip4ddr, port);					//create socket
	} else {
		//if(net_is_ip4ddr(ip4ddr) != 0) return -1;									//dns failed
		use_hardware_dns:
		sprintf((char *)cbuf, "ATPC=0,\"%s\",%d", host, port);				//create socket
	}
	restart_connect:
	len = if_net_command(ctx, (uint8 *)cbuf, rbuf, 200); 
	rbuf[len] = 0;
	if(strstr((char *)rbuf, "ERROR") != NULL) {
		//error connecting to server
		return -1;
	}
	if((ptr = (uint8 *)strstr((char *)rbuf, "con_id=")) != NULL) goto ready_connect;
	while(retry > 0) {
		len = if_net_data(ctx, (uint8 *)NULL, 0, rbuf, 2000); 
		rbuf[len] = 0;
		if(len == 0) {
			retry--;
			if(retry == 0) {
				if_net_command(ctx, (uint8 *)"ATPD=0", rbuf, 200);			//delete any available connections
				return -1;		//failed to connect
			}
			goto restart_connect;
		}
		if(strstr((char *)rbuf, "ERROR") != NULL) {
			//error connecting to server
			return -1;
		}
		if((ptr = (uint8 *)strstr((char *)rbuf, "con_id=")) != NULL) {
			//ready connect
			ready_connect:
			con_id = atoi((const char *)ptr+ 7);
			ctx->con_id = con_id;
#if RTL8710_TRANSPARENT_TRANSMISSION
			//if_net_sendstring(ctx, (uint8 *)"ATPU=1\r\n");
			if_net_command(ctx,  (uint8 *)"ATPU=1", rbuf, 1000);
			if(strstr((char *)rbuf, "ERROR:4") != NULL) {
				if_net_command(ctx,  (uint8 *)"ATPD=0", rbuf, 200);
				goto use_hardware_dns;
			}
			if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0) if_delay(200);
#endif
			return 0;
		}
	}
	return -1;
}

void if_net_tcp_close(net_context_p ctx) {
	uint8 buffer[72];
	if(ctx->con_id == 0) return;		//already closed
	//if_net_command(ctx, (uint8 *)"ATPK=0", buffer, 200);
#if RTL8710_TRANSPARENT_TRANSMISSION
	if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0) if_delay(200);
	if_net_sendstring(ctx, (uint8 *)"----\r\n"); 		//set auto receive off
#endif
	//sprintf((char *)buffer, "ATPK=0\r\n");
	//if_net_sendstring(ctx, buffer);
	sprintf((char *)buffer, "ATPD=0\r\n");
	if_net_sendstring(ctx, buffer);
	ctx->con_id = 0;					//reset con_id
}


uint16 if_net_tcp_send(net_context_p ctx, uint8 * payload, uint16 length, uint16 timeout) {
	uint8 lbuf[64];
	uint8 pbuf[512];
	uint16 len;
	uint16 i;
	uint32 wait = 200 * 1000;
	//start send request
#if !RTL8710_TRANSPARENT_TRANSMISSION
	sprintf((char *)pbuf, "ATPT=%d,%d:", length, ctx->con_id);			//transmit payload
	if_net_sendstring(ctx, pbuf);
	len = if_net_data(ctx, (uint8 *)payload, length, pbuf, 200);
#else
	for(i=0;i<length;i++) if_net_sendbyte(ctx, payload[i]);
#endif
	return length;
}

uint16 if_net_tcp_accept(net_context_p ctx, uint8 * host, uint32 timeout) {
	uint32 i;
	uint8 buffer[56];
	//use transparent receive
#if !RTL8710_TRANSPARENT_TRANSMISSION
	if_net_command(ctx, (uint8 *)"ATPK=1", buffer, 200); 				//set auto receive on
	for(i=0;i<4000000;i++) {  
		if(__HAL_USART_GET_FLAG((USART_HandleTypeDef *)ctx, USART_FLAG_RXNE) != RESET) {
			return 0;
		}
	}
#else
	return 0;
#endif
	return -1;
}

uint16 if_net_tcp_recv(net_context_p ctx, uint8 * response) {
	uint16 plen;
	uint16 len;
	uint16 i = 0;
	uint8 clen[10];
	uint16 dlen =0;
	uint8 * nextptr;
	uint8 * ptr;
	uint8 * dptr;
	len = 0;
	nextptr = response;
	//goto try_next;
	//while(1) {
#if RTL8710_TRANSPARENT_TRANSMISSION
	len = if_net_command(ctx, NULL, nextptr, 2000); 
#else
	while(1) {
		plen = if_net_data(ctx, NULL, 0, nextptr, 2000); 
		if(plen == 0) break;
		if((ptr = (uint8 *)strstr((const char *)nextptr, "[ATPR] OK,")) == NULL) break;
		dlen = atoi((const char *)ptr + 10);
		dptr = ptr;
		while(*dptr++ != ':');			//skip chars until colon found
		memcpy(nextptr, dptr, dlen);
		len += dlen;
		nextptr += dlen;
		if(dlen < 1400) break;			//end of packet (last packet should be less than 1400 bytes
	}
#endif
	return len;
}

//transmit tcp request and wait for response
uint16 if_net_tcp_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	uint16 len;
	if((ctx->state & IF_NET_STATE_CONNECTED) == 0) return 0;		//not connected
	NET_LOCK(ctx);
	if(if_net_tcp_connect(ctx, host, port) != 0)  {
		//error connecting to server
		NET_UNLOCK(ctx);
		return 0;
	}
	//start transmit
	if_net_tcp_send(ctx, payload, length, 200);
	len = 0;
	if(if_net_tcp_accept(ctx, host) == 0) {
		len = if_net_tcp_recv(ctx, response);
	}
	//close connection
	if_net_tcp_close(ctx);
	NET_UNLOCK(ctx);
	return len;		//unable to found +IPD prefix
}

uint16 if_net_udp_connect(net_context_p ctx, uint8 * host, uint16 port) {
	uint16 len;
	uint8 cbuf[90];
	uint8 ip4ddr[20];
	uint16 con_id;
	uint8 * ptr;
	if(net_is_ip4ddr(host) != 0) { 
		if(dns_translate(ctx, host, ip4ddr) != 0) return -1;			//try translate domain name address to ip address
		sprintf((char *)cbuf, "ATPC=1,%s,%d", ip4ddr, port);
	} else {
		//if(net_is_ip4ddr(ip4ddr) != 0) return -1;									//dns failed
		use_hardware_dns:
		sprintf((char *)cbuf, "ATPC=1,\"%s\",%d", host, port);
	}
	len = if_net_command(ctx, (uint8 *)cbuf, cbuf, 200); 
	cbuf[len] = 0;
	if(strstr((char *)cbuf, "ERROR") != NULL) {
		//error connecting to server
		return -1;
	}
	if((ptr = (uint8 *)strstr((char *)cbuf, "con_id=")) != NULL) {
		//ready connect
		goto ready_connect;
	}
	//try waiting until ready
	len = if_net_command(ctx, (uint8 *)NULL, cbuf, 2000); //wait till ready
	cbuf[len] = 0;
	if((ptr = (uint8*)strstr((char *)cbuf, "con_id=")) != NULL) {
		//ready connect
		ready_connect:
		con_id = atoi((const char *)ptr+ 7);
		ctx->con_id = con_id;
#if RTL8710_TRANSPARENT_TRANSMISSION
		if_net_sendstring(ctx, (uint8 *)"ATPU=1\r\n");
		if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0) if_delay(200);
#endif
		return 0;
	}
	return -1;
}

void if_net_udp_close(net_context_p ctx) {
	uint8 buffer[28];
	if(ctx->con_id == 0) return;		//already closed
#if RTL8710_TRANSPARENT_TRANSMISSION
	if((ctx->state & IF_NET_STATE_CRITICAL_SECTION) == 0) if_delay(200);
	if_net_sendstring(ctx, (uint8 *)"----\r\n"); 		//set auto receive off
#endif
	//sprintf((char *)buffer, "ATPK=0\r\n");
	//if_net_sendstring(ctx, buffer);
	sprintf((char *)buffer, "ATPD=0\r\n");
	if_net_sendstring(ctx, buffer);
	ctx->con_id = 0;					//reset con_id
}

uint16 if_net_udp_send(net_context_p ctx, uint8 * payload, uint16 length, uint16 timeout) {
	uint8 lbuf[64];
	uint8 pbuf[512];
	uint16 len;
	uint16 i;
	uint32 wait = 200 * 1000;
	//start send request
#if !RTL8710_TRANSPARENT_TRANSMISSION
	sprintf((char *)pbuf, "ATPT=%d,%d:", length, ctx->con_id);			//transmit payload
	if_net_sendstring(ctx, pbuf);
	len = if_net_data(ctx, (uint8 *)payload, length, pbuf, 200);
#else
	for(i=0;i<length;i++) if_net_sendbyte(ctx, payload[i]);
#endif
	return length;
}

uint16 if_net_udp_accept(net_context_p ctx, uint8 * host) {
	return if_net_tcp_accept(ctx, host, 5000);
}

uint16 if_net_udp_recv(net_context_p ctx, uint8 * response) {
	uint16 plen;
	uint16 len;
	uint16 i = 0;
	uint8 clen[10];
	uint8 * nextptr;
	uint8 * ptr;
	len = 0;
	nextptr = response;
	//while(1) {
#if RTL8710_TRANSPARENT_TRANSMISSION
	plen = if_net_command(ctx, NULL, nextptr, 8000); 
#else
	plen = if_net_command(ctx, NULL, nextptr, 8000);
#endif
	return plen;
}

//transmit udp reqeuest and wait for response
uint16 if_net_udp_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	uint16 len;
	if((ctx->state & IF_NET_STATE_CONNECTED) == 0) return 0;		//not connected
	NET_LOCK(ctx);
	if(if_net_udp_connect(ctx, host, port) != 0)  {
		//error connecting to server
		NET_UNLOCK(ctx);
		return 0;
	}
	//start transmit
	//if_net_escape_string(payload, length, response);
	if_net_udp_send(ctx, payload, length, 200);
	len = 0;
	//if(if_net_udp_accept(ctx, host) == 0) {
	len = if_net_udp_recv(ctx, response);
	//}
	if_net_udp_close(ctx);
	NET_UNLOCK(ctx);
	return len;		//unable to found +IPD prefix
}

static void if_net_tick(net_context_p ctx) {
	uint8 ssidname[256];
	if(NET_BUSY(ctx)) return;
	if_net_is_connected(ctx, ssidname);
}

void if_net_sleep(net_context_p ctx) {
	uint8 netbuf[256];
	//if_net_command(ctx, (uint8 *)"ATSP=r", netbuf, 200); 				//timeout 5 second
	//if_net_sendstring(ctx, (uint8 *)"ATSP=r\r\n");
}

void if_net_wake(net_context_p ctx) {
	uint8 netbuf[256];
	//if_net_command(ctx, (uint8 *)"ATSP=a", netbuf, 200); 				//timeout 5 second
}

void if_net_enable(net_context_p ctx) {
	GPIO_SetBits(GPIOB, GPIO_PIN_1);				//enable device
}

void if_net_disable(net_context_p ctx) {
	GPIO_ResetBits(GPIOB, GPIO_PIN_1);				//disable device
}

#if SHARD_RTOS_ENABLED 

struct net_async_params {
	uint8 type;
	void * param1;
	void * param2;
	void * param3;
	void * param4;
	void * param5;
} net_async_params;

uint8 if_net_ssid_join_async(net_context_p ctx, uint8 * ssidname, uint8 * username, uint8 * password) {
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

uint8 if_net_ssid_list_async(net_context_p ctx) {
	struct net_async_params req;
	os_message * msg;
	uint8 num_ssid;
	req.type = 1;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &num_ssid));
	os_delete_message(msg);
	return num_ssid;
}

uint16 if_net_tcp_transmit_async (net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 3;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_udp_transmit_async(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 4;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

uint16 if_net_ssl_transmit_async(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) {
	struct net_async_params req;
	os_message * msg;
	uint16 ret;
	req.type = 5;
	req.param1 = host;
	req.param2 = (void *)(unsigned)port;
	req.param3 = payload;
	req.param4 = (void *)(unsigned)length;
	req.param5 = response;
	msg = os_send_message(os_find_task_by_name("net"), os_create_message(ctx, &req, sizeof(net_async_params), &ret));
	os_delete_message(msg);
	return ret;
}

static void if_net_deep_sleep(net_context_p ctx, uint16 ms) {
	uint8 netbuf[256];
	//sprintf((char *)netbuf, "AT+GLSP=%d", ms);
	//if_net_command(ctx, netbuf, netbuf, 200); 				//timeout 5 second
}

void if_net_task() {
	#define TICK_LIMIT(x)		(x*5)
	os_message * msg;
	net_context_p ctx = os_get_context();
	uint16 counter = 0;
	uint8 ssidname[256];
	struct net_async_params * params;
	while(1) {
		msg = os_dequeue_message();
		if(msg != NULL) {
			ctx = msg->context;
			if(ctx == NULL) goto abort_operation;
			if(msg->reqlen == 0) goto abort_operation;
			params = msg->request;
			switch(params->type) {
				case 0:		//join
					((uint8 *)msg->response)[0] = if_net_ssid_join(ctx, params->param1, params->param2, params->param3);
					msg->reslen = sizeof(uint8);
					break;
				case 1:		//list
					((uint8 *)msg->response)[0] = if_net_ssid_list(ctx);
					msg->reslen = sizeof(uint8);
					break;
				case 3:		//tcp transmit
					((uint16 *)msg->response)[0] = if_net_tcp_transmit(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5);
					msg->reslen = sizeof(uint16);	
					break;
				case 4:		//udp transmit
					((uint16 *)msg->response)[0] = if_net_udp_transmit(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5);
					msg->reslen = sizeof(uint16);
					break;
				case 5:		//ssl transmit
					((uint16 *)msg->response)[0] = if_ssl_transmit(ctx, params->param1, (uint16)params->param2, params->param3, (uint16)params->param4, params->param5);
					msg->reslen = sizeof(uint16);
					break;
				default: break;
			}
			abort_operation:
			os_dispatch_reply(msg);
		}
		//NET_LOCK(ctx);
		//if_net_deep_sleep(ctx, 3000);
		//NET_UNLOCK(ctx);
		os_wait(300);
		counter ++;
		if(counter >= TICK_LIMIT(30)) {		//every 30 second
			NET_LOCK(ctx);
			if_net_is_connected(ctx, ssidname);
			NET_UNLOCK(ctx);
			counter =0;
		}
	}
}
#endif

void if_net_init(void * display, net_context_p ctx) {
	//net_ssid_p list;
	uint16 len;  
	//USART_InitTypeDef USART_InitStructure;
	uint8 netbuf[4096];
	rtl8710_init_config(ctx, 38400);			//initialize pin and usart1 (ready to transmit data)
	memset(ctx->ipv4, 0, 4);
	memset(ctx->mac, 0, 6);
	//disable echo and debug message
	if_delay(250);
	if_net_command(ctx, (uint8 *)"ATSR", netbuf, 200); 								//reset device
	if_delay(2000);																				//wait for device to ready
	if_net_command(ctx, (uint8 *)"ATSE=0,0x0,0x0", netbuf, 200); 				//disable echo
	if_net_command(ctx, (uint8_t *)"ATPW=1", netbuf, 200); 						//set station mode
//////////////ERROR RTL COULD LATER CANNOT BE ACCESSED//////////////////////////	
	//change baudrate
	if_net_command(ctx, (uint8 *)"ATSU=38400,8,1,0,0,0", netbuf, 200); 
	//rtl8710_init_config(ctx, 115200);			//initialize pin and usart1 (ready to transmit data)
	//rtl8710_init_config(ctx, 115200);
/////////END ERROR RTL COULD LATER CANNOT BE ACCESSED//////////////////////////
#if SHARD_RTOS_ENABLED 
	ctx->join = if_net_ssid_join_async;
	ctx->list = if_net_ssid_list_async;
	//ctx->send_http = if_net_http_send;
	ctx->send_tcp = if_net_tcp_transmit_async;
	ctx->send_udp = if_net_udp_transmit_async;
	ctx->send_ssl = if_net_ssl_transmit_async;
	os_create_task(ctx, if_net_task, "net", 40, 16384);
#else
	//check for saved ssid name
	if_net_ssid_list(ctx);
	ctx->join = if_net_ssid_join;
	ctx->list = if_net_ssid_list;
	//ctx->send_http = if_net_http_send;
	ctx->send_tcp = if_net_tcp_transmit;
	ctx->send_udp = if_net_udp_transmit;
	//if_net_list_network(ctx, &list);
	if_timer_create(if_net_tick, ctx, 30);		//tick every 30 second
#endif
}

#endif 			//END OF RTL8710 MODULE