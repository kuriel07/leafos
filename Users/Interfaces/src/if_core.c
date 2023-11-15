#ifdef WIN32
//#include "stdafx.h"
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\defs.h"
//#include "..\inc\curl\curl.h"
#include "..\inc\if_apis.h"

CONST net_protocol g_strRegisteredProtocol[] = {
	{ "http", 80, NETP_TRANSPARENT, http_send },
	{ "coap", 5683, NETP_TRANSPARENT, coap_send },
#if SHARD_SSL_SUPPORT
	{ "https", 443, NETP_USE_SSL, https_send },
#endif
	{ NULL, 0, 0, NULL }						//end mark
};

CONST net_protocol g_strDefaultProtocol = { "raw", 80, NETP_TRANSPARENT, raw_send };

net_request_p net_request_struct(net_request_p req, uint16 type, uint8 * uri, uint16 port, ssl_cert_p cert) {
	req->type = type;
	req->uri = uri;
	req->port = port;
	req->cert = cert;
	return req;
}

net_request_p net_request_create(uint16 type, uint8 * uri, uint16 port, ssl_cert_p cert) {
	net_request_p req = (net_request_p)os_alloc(sizeof(net_request));
	return net_request_struct(req, type, uri, port, cert);
}

//convert from arg string to get string request
uint16 net_get_uri_string(uint8 * request, uint8 * outbuf) {
	//convert to format specified by rfc3986
	uint16 len = strlen((const char *)request);
	uint16 i,j,k;
	uint8 c;
	for(i=0,j=0;i<len;i++) {
		c = request[i];
		if(c >= '0' && c<='9') outbuf[j++]=c;
		else if(c >= 'a' && c<='z')  outbuf[j++]=c;
		else if(c >= 'A' && c<='Z')  outbuf[j++]=c;
		else {
			switch(c) {
				case '_':
				case '-':
				case '.':
				case '?':
				case '=':
				case '&':
				case '#':
				case '@':
				case '!':
				case '~':
				case '/':
				case '\\':
					outbuf[j++]=c;	
					break;
				default:
					sprintf((char *)outbuf+j, "%%%02X", c);
					j+=3;
				break;
			}
		}
	}
	outbuf[j++] = 0;
	return j;
}

net_protocol_p net_get_protocol(uint8 * url) { 
	uint16 i,j,k,n=0;
	uint8 cphr = 0xFF;
	uint8 strport[7];
	uint8 protocol[20];
	uint8 slsctr = 0;
	uint16 ulen = strlen((const char *)url);
	memset(strport, 0, sizeof(strport));
	for(i=0;i<ulen && i<19;i++) {
		if(url[i] == ':') break;
		protocol[i] = url[i];
	}
	protocol[i++] = 0;
	j = 0;
	while(g_strRegisteredProtocol[j].name != NULL) {
		if(strcmp((char *)protocol, g_strRegisteredProtocol[j].name) == 0) {
			return (net_protocol_p)&g_strRegisteredProtocol[j];
		}
		j++;
	}
	if(strstr((const char *)url, "://") != NULL) return NULL;				//unsupported protocol
	return (net_protocol_p)&g_strDefaultProtocol;								//in case protocol not defined, use raw protocol
}

uint8 * net_decode_url(uint8 * url, uint16 * port, uint8 * host) { 
	uint16 i = 0, j, k, n=0;
	uint8 cphr = 0xFF;
	uint8 strport[7];
	uint8 slsctr = 0;
	uint8 * ptr;
	uint16 ulen = strlen((const char *)url);
	memset(strport, 0, sizeof(strport));
	if((ptr = (uint8 *)strstr((char *)url, "://")) != NULL) {
		//contain protocol prefix, skip protocol prefix
		i = (ptr + 3) - url;
	}
	j = 0;
	//check if port already defined in url
	for(;i<ulen && slsctr < 1;i++) {
		if(url[i] == '/') { slsctr ++; continue; }
		if(url[i] == '\\') { slsctr ++; continue; }
		if(url[i] == ':') cphr = 1;
		if(slsctr == 3) break;
		if(url[i] >='0' && url[i] <='9' && cphr < 7) strport[cphr++] = url[i];
		if(cphr == 0xFF) host[n++] = url[i];
	}
	host[n++] = 0;
	if(strlen((const char *)strport) != 0) {
		port[0] = atoi((const char *)strport);
	}
	if(slsctr < 1) return (uint8 *)"/";		//return slash, end of url
	return url + i -1;
}

uint8 net_text_2_ip4ddr(uint8 * text, uint8 ip_addr[4]) {
	uint8 num[9];
	uint8 cphr = 0xFF;
	uint16 len = strlen((const char *)text);
	uint16 i,j=0,k=0;
	memset(ip_addr, 0, 4);
	memset(num, 0, sizeof(num));
	for(i=0;i<len && k<4;i++) {
		if(text[i] >='0' && text[i] <='9' && j < sizeof(num)) num[j++] = text[i];
		else if(text[i] == '.') {
			ip_addr[k++] = (uint8)atoi((const char *)num);
			memset(num, 0, sizeof(num));
			j = 0;
		} else return -1;
	}
	if(k >= 4) return -1;			//too many dot
	ip_addr[k++] = (uint8)atoi((const char *)num);
	if(k == 4) {
		return 0;			//valid ip4 address
	}
	return -1;			//valid domain name address
}

//check whether an host name is already in ip address format, eliminating translation cost
uint8 net_is_ip4ddr(uint8 * host) {
	uint8 num[9];
	uint8 cphr = 0xFF;
	uint16 len = strlen((const char *)host);
	uint16 i,j=0,k=0;
	memset(num, 0, sizeof(num));
	for(i=0;i<len && k<4;i++) {
		if(host[i] >='0' && host[i] <='9' && j < sizeof(num)) num[j++] = host[i];
		else if(host[i] == '.') {
			//ipddr[k++] = atoi((const char *)num);
			memset(num, 0, sizeof(num));
			j = 0;
			k++;
		} else return -1;
	}
	if(k >= 4) return -1;			//too many dot
	//ipddr[k++] = atoi((const char *)num);
	k++;
	if(k == 4) {
		return 0;			//valid ip4 address
	}
	return -1;			//valid domain name address
}

static sim_entry_p g_sim_entries = NULL;
void sim_register_interface(uint8 * name, void * ctx,  uint16 (* handler)(void * ctx, sim_query_p query)) {
	sim_entry_p iterator = g_sim_entries;
	sim_entry_p sim = os_alloc(sizeof(sim_entry));
	if(sim == NULL) return;
	strncpy((char *)sim->name, (const char *)name, 15);
	sim->handler = handler;
	sim->ctx = ctx;
	sim->next = NULL;
	if(g_sim_entries == NULL) {
		g_sim_entries = sim;
	} else {
		while(iterator->next != NULL) iterator = iterator->next;
		iterator->next = sim;
	}
}

uint16 sim_command_query(uint8 * name, sim_query_p query) {
	sim_entry_p iterator = g_sim_entries;
	uint16 ret = 0;
	while(iterator != NULL) {
		if(strncmp((const char *)iterator->name, (const char *)name, 15) == 0) {
			ret = iterator->handler(iterator->ctx, query);
		}
		iterator = iterator->next;
	}
	return ret;
}

uint32 g_int_flag = 0;
uint32 if_pwr_get_interrupt_source() {
	return g_int_flag;
}

void if_pwr_set_interrupt_source(uint32 flag) {
	g_int_flag |= flag;
}

void if_pwr_clear_interrupt_source() {
	g_int_flag = 0;
}

unsigned _dma_xfer_completed = TRUE;
void DMA2_Stream0_IRQHandler(void) {
	if((DMA2->LISR)&DMA_LISR_TCIF0)
	{
					//set finished to 1
		_dma_xfer_completed=TRUE;
					//clear transfer complete interrupt
		DMA2->LIFCR=DMA_LIFCR_CTCIF0;
	}
	if((DMA2->LISR)&DMA_LISR_HTIF0)
	{
		DMA2->LIFCR=DMA_LIFCR_CHTIF0;
	}
}
static void HAL_DMA_TransferCompleted(DMA_HandleTypeDef *hdma) {
	uint32_t err = HAL_DMA_GetError(hdma);
	if(err == 0) {
		err = 0;
	}
}

void dma_memcpy(void * dst, void * src, size_t sz) {
	
	memcpy(dst, src, sz);
	return;
	DMA_HandleTypeDef dma_x;
  __HAL_RCC_DMA2_CLK_ENABLE();
  dma_x.Instance = DMA2_Stream0;
  dma_x.Init.Channel = DMA_CHANNEL_0;
  dma_x.Init.Direction = DMA_MEMORY_TO_MEMORY;//DMA_MEMORY_TO_PERIPH;
  dma_x.Init.Mode = DMA_NORMAL;
  dma_x.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  dma_x.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  dma_x.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  //source
  dma_x.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  dma_x.Init.PeriphInc = DMA_PINC_ENABLE;
  dma_x.Init.PeriphBurst = DMA_PBURST_INC4;
  //destination
  dma_x.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  dma_x.Init.MemBurst = DMA_MBURST_SINGLE ;
  dma_x.Init.MemInc = DMA_MINC_ENABLE;
  
  HAL_DMA_Init(&dma_x);
  HAL_DMA_IRQHandler(&dma_x);
  HAL_DMA_RegisterCallback(&dma_x, HAL_DMA_XFER_CPLT_CB_ID, HAL_DMA_TransferCompleted);
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);
	
	dma_wait();
	HAL_DMA_Abort(&dma_x);
	_dma_xfer_completed = FALSE;
	HAL_DMA_Start_IT(&dma_x, (uint32_t)src, (uint32_t)dst, sz/4 );
	//	while(__HAL_DMA_GET_FLAG(&dma_x, DMA_FLAG_TCIF0_4) != 0) { os_wait(15); }
}

void dma_wait() {
	while(_dma_xfer_completed == FALSE) {
		//os_wait(15);
	}
}

static uint32 if_file_read_mem(if_file * ctx, uint32 offset, uint8 * data, uint32 size) {
	if((offset + size) >= ctx->size) size = ctx->size - offset;
	memcpy(data, (uint8 *)ctx->handle + offset, size);
	return size;
}

static uint32 if_file_write_mem(if_file * ctx, uint32 offset, uint8 * data, uint32 size) {
	if((offset + size) >= ctx->size) size = ctx->size - offset;
	memcpy((uint8 *)ctx->handle + offset, data, size);
	return size;
}

static void if_file_close_mem(if_file * ctx) {
	
}

if_file * if_file_open_mem(uint8 * address, uint32 size) {
	if_file * file = os_alloc(sizeof(if_file));
	if(file == NULL) return NULL;
	file->handle = address;
	file->offset = 0;
	file->size = size;
	file->read = if_file_read_mem;
	file->write = if_file_write_mem;
	file->close = if_file_close_mem;
	return file;
}

uint32 if_file_fread(uint8 * data, size_t elem, size_t count, if_file * file) {
	uint32 read = file->read(file, file->offset, data, count * elem);
	file->offset += read;
	return read;
}

uint32 if_file_fwrite(uint8 * data, size_t elem, size_t count, if_file * file) {
	uint32 wrote = file->write(file, file->offset, data, count * elem);
	file->offset += wrote;
	return wrote;
}

uint32 if_file_fseek(if_file * file, uint32 offset, uint8 mode) {
	file->offset = offset;
}

void if_file_fclose(if_file * file) {
	file->close(file);
	os_free(file);
}

void if_system_reset() {
	NVIC_SystemReset();
	while(1);
}