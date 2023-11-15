/*!\file 			if_apis.h
 * \brief     	interface (hardware/software) abstraction layer
 * \details   	interface (hardware/software) abstraction layer
 * \author    	AGP
 * \version   	1.0
 * \date      	2016
 * \copyright 	OrbLeaf Technology
 * \pre       	
\verbatim	
********************************************************************
1.0
 * initial release (2015.12.XX)
1.1
* changed : ported from STM32F10x to STM32F40x (2016.07.17)
* changed : datetime format from tick based (STM32F10x) to calendar based (STM32F40x)
* added : SSL support using wolfSSL, still need to increase SSL performance (2016.07.20)
* added : libpng, zlib support for image decoding (2016.11.24)
* fixed : update firmware for multitasking (2017.03.21)
* fixed : ssl transmit for multitasking support (2017.04.15)
* fixed : battery if_pwr_get_batt_percent() from ADC1.channel8 (PB0) to ADC1.channel18 (VBAT) (2017.04.30)
* added: NTP protocol for synchronization with time server (2017.06.05)
* fixed: RTC bug affecting Touch Interrupt, calling HAL_RTC_Init on if_time_init (prev if_time_set) (2017.06.08)
* added: RFID interface for PICC communication, RC522 Micore series, SPI communication (2017.06.17)
* added: driver for PN532 NFC interface (2017.09.18)
* changed: ILI9341, initialization sequence (2017.10.27)
* added: capacitive driver based on FT6236, I2C HAL driver STMCube (2017.10.29)
* added: libjpeg-turbo support for image decoding, ui_image (2017.11.20)
* changed: PN532 low level interface to I2C HAL driver STMCube (2017.11.24)
* added: ui_picture_create a new component to display image, supporting png and jpg (2017.12.08)
* added: BLE interface driver for HM-11 (2018.02.15)
* modified: if_net to support BLE for ESP32 -> API defined by if_ble.h (2018.03.23)
* added: System Interface Management (SIM) query (2018.05.04)
* modified: http support transfer-encoding:chunk (2018.05.08)
* added: support for FSMC interface, STM32F7-ILI934x (2018.07.24)
* added: SSD1351 driver for 128x128 OLED display (2018.10.15)
********************************************************************
\endverbatim
 */

#ifndef IF_APIS__H
#define IF_APIS__H
#ifdef WIN32
//#include <winscard.h>
#endif
#ifndef _DEFS__H
#include "defs.h"
#endif
#include "if_file.h"
#include "if_card.h"
#include "if_touch.h"
#include "if_net.h"
#include "if_timer.h"
#include "if_usb.h"
#include "if_ext.h"
#include "if_flash.h"
#include "if_ssl.h"
#include "if_png.h"
#include "if_rfid.h"
#include "if_pwm.h"
#include "if_nfc.h"
#include "if_ble.h"
#include "if_gps.h"
#include "if_sdram.h"
#include "if_audio.h"
#include "if_storage.h"
#include "..\..\gui\inc\ui_core.h"
#include "..\..\gui\inc\ui_item.h"
#include "..\..\gui\inc\ui_icon.h"
#include "..\..\gui\inc\ui_textbox.h"
#include "..\..\gui\inc\ui_toolbox.h"
#include "..\..\gui\inc\ui_signal.h"
#include "..\..\gui\inc\ui_label.h"
#include "..\..\gui\inc\ui_button.h"
#include "..\..\gui\inc\ui_alert.h"
#include "..\..\gui\inc\ui_combo.h"
#include "..\..\gui\inc\ui_scroll.h"
#include "..\..\gui\inc\ui_setting.h"
#include "..\..\gui\inc\ui_datetime.h"
#include "..\..\gui\inc\ui_qrcode.h"
#include "..\..\gui\inc\ui_gauge.h"
#include "..\..\gui\inc\ui_picture.h"
#include "..\..\gui\inc\ui_window.h"
#include "..\..\gui\inc\ui_tasker.h"
#if SHARD_RTOS_ENABLED
#include "..\..\core\inc\os.h"
#endif

//scard constant definition
#define SCARD_EXCLUSIVE		1
#define SCARD_SHARED			2
#define SCARD_ERROR(x)			(x == (uint16)-1)

#define NET_BUFFER_SIZE			32000

//request packet mode
//mode[0:2]  	=> 8 type of request methods (HEADER,GET,POST,PUT,DELETE,....)
//mode[3]  	=> transport protocol (1=UDP,0=TCP)
//mode[4-6]	=> 8 type of payload format (0=Plain,1=JSON,2=OWB)
#define IF_HTTP_TYPE_HEADER		0
#define IF_HTTP_TYPE_GET		1
#define IF_HTTP_TYPE_POST		2
#define IF_HTTP_TYPE_PUT		3
#define IF_HTTP_TYPE_DELETE		4

#define IF_TRANSPORT_TCP					0x00
#define IF_TRANSPORT_UDP					0x08

#define IF_PAYLOAD_PLAIN		0x00
#define IF_PAYLOAD_JSON			0x10
#define IF_PAYLOAD_OWB			0x20

#define IF_KRONOS_SIGNATURE_LEN		0x20

#define IF_INT_WAKE				0x8000
#define IF_INT_TOUCH			0x0001		
#define IF_INT_CARD				0x0002
#define IF_INT_NET				0x0004
#define IF_INT_BLE				0x0003

//tcp typedef definition
typedef struct tcp_context tcp_context;
typedef struct tcp_context * tcp_context_p;
typedef struct net_protocol * net_protocol_p;
typedef struct net_request * net_request_p;
typedef struct datetime * datetime_p;
typedef struct sim_query * sim_query_p;
typedef struct sim_entry * sim_entry_p;

#define NETP_TRANSPARENT		0x00
#define NETP_USE_SSL				0x01

typedef struct net_request {
	uint8 type;
	uint8 * uri;
	uint16 port;
	void * cert;
} net_request;

typedef struct net_protocol {
	const char * name;
	uint16 port;				//default port
	uint16 type;
	uint16 (* send)(net_context_p ctx, net_request * request, uint8 * headers, uint8 * payload, uint16 length, uint8 * response);
} net_protocol;

extern uint8 gba_net_buffer[NET_BUFFER_SIZE];
#define gba_net_bufstream (gba_net_buffer + (NET_BUFFER_SIZE >> 1))

typedef struct sim_entry {
	uint8 name[15];
	uint16 (* handler)(void *, sim_query_p query);
	void * ctx;
	struct sim_entry * next;
} sim_entry;

typedef struct sim_query {
	uint8 * cmd;
	uint8 * buffer;
	uint16 buf_size;
	uint8 argc;
	uint8 * argv[16];
} sim_query;

//image info
typedef struct bfd_info {
	uint8 status;
	uint32 len;
	uint8 signature[IF_KRONOS_SIGNATURE_LEN];
} bfd_info;

//scard apis definition
void if_card_init(void * display, scard_context * ctx);
uint8 if_card_state(scard_context * ctx);
uint8 if_card_list(scard_context * ctx);
uint8 if_card_connect(scard_context * ctx, BYTE mode, BYTE * atr);
uint16 if_card_send(scard_context * ctx, BYTE * c_apdu, uint8 length, BYTE * r_apdu);
uint8 if_card_disconnect(scard_context * ctx, BYTE mode);

//ui apis definition
void if_display_wake(gui_handle_p ctx);
void if_display_sleep(gui_handle_p ctx);
uint8 if_gui_init(gui_handle_p handle, uint8 orientation, uint8 brightness);
void if_gui_switch_orientation(gui_handle_p display, uint8 mode) ;
void * if_gui_create_object(size_t objsize, DWORD mode, DWORD color, uchar * text) ;
void if_gui_clearscreen(gui_handle_p display, uint32 color);
uint8 if_gui_add_object(gui_handle_p display, void * object) ;
void if_gui_delete_object(gui_handle_p display, BYTE id);
void * if_gui_present(gui_handle_p display);
void if_gui_switch_orientation(gui_handle_p display, uint8 mode) ;

//file apis definition
if_file * if_file_open_mem(uint8 * address, uint32 size);
uint32 if_file_fread(uint8 * data, size_t elem, size_t count, if_file * file);
uint32 if_file_fwrite(uint8 * data, size_t elem, size_t count, if_file * file);
uint32 if_file_fseek(if_file * file, uint32 offset, uint8 mode);
void if_file_fclose(if_file * file);

//system apis definition
void if_sys_init(void * sys);
void if_sys_sleep(void) ;
void if_sys_wake(void) ;
uint32 if_sys_tick(void);
#if SHARD_RTOS_ENABLED
#define if_delay(x) os_wait(x)
#else
void if_delay(uint32 ms);
#endif
uint8 if_pwr_get_batt_percent(void) ;
uint8 if_pwr_is_charging(void);
uint32 if_pwr_get_interrupt_source(void);
void if_pwr_set_interrupt_source(uint32 flag);
void if_pwr_clear_interrupt_source(void);

//configuration APIs (non-volatile)
void if_config_write(void * data, uint16 size);
uint8 if_config_read(void * data, uint16 size);

//RTC APIs
void if_time_init(void); 
uint8 if_time_is_running(void);
void if_time_set(datetime_p val);
uint32 if_time_get(datetime_p val);

//error handling
uint32 if_get_last_error(uint8 * buffer);
void if_set_last_error(uint32 code, uint8 * buffer, uint8 size) ;
void if_clear_error(void);
uint32 if_download_update(net_context_p ctx, uint8 * url, uint32 * address, uint8 * sign);
uint8 if_check_loadfile(uint8 * offset, uint8 * signature, uint32 length);
void if_system_reset(void) ;
void __svc(1) 	if_load_new_firmware(void);

//system interface management APIs
void sim_register_interface(uint8 * name, void * ctx, uint16 (* handler)(void * ctx, sim_query_p query)) ;
uint16 sim_command_query(uint8 * name, sim_query_p query);


//dma operation
void dma_memcpy(void * dst, void * src, size_t sz);
void dma_wait(void);

//interface misc
uint16 net_get_uri_string(uint8 * request, uint8 * outbuf);
net_protocol_p net_get_protocol(uint8 * url) ;
uint8 * net_decode_url(uint8 * url, uint16 * port, uint8 * host);
uint8 net_text_2_ip4ddr(uint8 * text, uint8 ip_addr[4]) ;
uint8 net_is_ip4ddr(uint8 * host) ;
net_request_p net_request_struct(net_request_p, uint16 type, uint8 * uri, uint16 port, ssl_cert_p cert);
net_request_p net_request_create(uint16 type, uint8 * uri, uint16 port, ssl_cert_p cert);
//ether apis definition
uint8 dns_translate(net_context_p ctx, uint8 * host, uint8 * ipstr);
uint8 netbios_translate(net_context_p ctx, uint8 * host, uint8 * ipstr);
uint8 ntp_get_time( net_context_p ctx, uint8 * server, datetime * dtime);			//NTP request
uint16 udp_send(net_context_p ctx, uint8 * host, uint16 port, uint8 * request, uint16 len, uint8 * response);
uint16 tcp_send(net_context_p ctx, uint8 * host, uint16 port, uint8 * request, uint16 len, uint8 * response);
//ether supported protocol
uint16 http_parse_response(uint8 * response, uint16 len, uint8 * outbuf);
//uint16 https_fetch(ssl_handle_p ctx, uint8 type, uint8 * url, uint16 port, uint8 * payload, uint16 length, uint8 * response);
uint16 raw_send(net_context_p ctx, net_request_p req, uint8 * headers, uint8 * payload, uint16 length, uint8 * response) ;
uint16 http_send(net_context_p ctx, net_request_p req, uint8 * headers, uint8 * payload, uint16 length, uint8 * response);
uint16 coap_send(net_context_p ctx, net_request_p req, uint8 * headers, uint8 * payload, uint16 length, uint8 * response);
uint16 https_send(net_context_p ctx, net_request_p req, uint8 * headers, uint8 * payload, uint16 length, uint8 * response);

#endif


