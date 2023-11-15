/*!\file 		net.h
 * \brief     	network driver 
 * \details   	network driver for communicating with Wi-Fi module (changed from only ESP8266)
 * \author    	AGP
 * \version   	2.0
 * \date      	2016
 * \pre       	
 * direct access to CMSIS
 * use usart1 (PA9 = TX, PA10 = RX, PA8 = RST)
\verbatim	
********************************************************************
1.0
 * initial release (2016.03.08)
 * successfully connected to AP (2016.03.12, roshi's birthday)
 * use GPIOA.9 TX, GPIOA.10 RX, GPIOA.8 RST, GPIOA.11 ChipEn
 * added if_ned_ssid_check to check ssid name from ssid list (2016.03.17)
 * changed if_net_ssid_join using state to detect WIFI CONNECTED, FAIL and OK (2016.03.17)
 * fixed when receiving more than one data block (+IPD) (2016.03.17)
 * changed: support raw data transmit via TCP (2016.05.13)
 * switched: net layering http->tcp (2016.05.14)
 * changed: toolkit API for accessing tcp request STK_TAG_URL, STK_TAG_TRANSPORT_LEVEL, STK_TAG_CHANNEL_DATA (2016.05.14)
 * added: supported http POST method (2016.05.14)
 * split to: if_net_tcp_connect, accept, send and recv (2016.06.08)
 * added : if_net_udp_connect, accept, send and recv, tested (2016.06.14)
 * added : if_net_wake, if_net_sleep for power saving (2016.12.01)
 * added : critical mode, during if_net_send_command (2017.03.21)
 * added : if_net_disable, if_net_enable to shutdown/poweron wifi module (2017.05.12)
 * changed: added support for RTL8710 through MOD_RTL8710, compile switch (2017.05.24)
 * fixed: RTL8710 tcp_connect, tcp_accept and net_init (2017.05.25)
 * fixed: RTL8710 ssl operation, using transparent transmission (2017.05.27)
1.1
 * changed: all data received now handled by interrupt (async - non-blocking), untested (2018.01.13)
 * added: exception, prepare and finish handler with anti-nested (2018.01.15)
 * added: added support for socket operation asynchronous supporting TCP, UDP and SSL/TLS (2018.02.18)
2.0
 * added: net_conn datatype, mode argument on socket open, incompatible with ver 1.x (2018.04.22)
 * fixed: if_net_accept and wifi task for handling larger amount of data (2018.05.08)
********************************************************************
\endverbatim
 */
 
#include "..\..\defs.h"
#include "..\..\config.h"
#ifndef IF_NET__H
#define IF_NET__H

//net configuration
#define NET_USE_CHANNEL				1
#define NET_MAX_CHANNEL				4

#define NET_TYPE_WIFI				0x02
#define NET_TYPE_GSM				0x01

#define IF_NET_STATE_CONNECTED					0x0001
#define IF_NET_STATE_IP_ASSIGNED				0x0002
#define IF_NET_STATE_OPENED						0x0004
#define IF_NET_STATE_READY						0x0008
#define IF_NET_STATE_DISABLED					0x0200
#define IF_NET_STATE_ENABLED					0x0400
#define IF_NET_STATE_CRITICAL_SECTION			0x0800
#define IF_NET_STATE_BUSY						0x1000
#define IF_NET_STATE_SLEEP						0x2000
#define IF_NET_STATE_BLE_AVAIL					0x4000
#define IF_NET_STATE_INITIALIZED				0x8000
#define IF_NET_STATE_GPS_AVAIL					0x0080

#define IF_NET_MAX_SSID_NAME			50
#define IF_NET_MAX_SSID_PASSWORD	30
#define NET_SSIDREC_MAGIC				0xEE

#define NET_STAT_BUF_RDY			0x01
#define NET_STAT_BUF_FULL			0x02
#define NET_STAT_CLEAR				0x00

#define NET_CONN_CONNECTED				0x1
#define NET_CONN_READY					0x2		//ready to accept data
#define NET_CONN_LISTEN					0x3
#define NET_CONN_CLOSED					0x0
#define NET_CONN_BUFRDY					0x4		//buf is ready to read
#define NET_CONN_BUFLOW					0x8
#define NET_CONN_BUSY					0x10

#define NET_LISTEN						0x10
#define NET_TRANSMIT					0x00
#define NET_TYPE_TCP					0x00
#define NET_TYPE_UDP					0x01
#define NET_TYPE_SSL					0x04

#define NET_SUCCESS						0
#define NET_ERR_JOIN						1
#define NET_ERR_NO_IP						2

#define NET_BUSY(ctx)						(ctx->state & IF_NET_STATE_BUSY)
#define NET_LOCK(ctx)						((ctx->state |= IF_NET_STATE_BUSY))
#define NET_UNLOCK(ctx)						((ctx->state &= ~IF_NET_STATE_BUSY))

#define NET_LOCK_CRITICAL(ctx)						((ctx->state |= IF_NET_STATE_BUSY | IF_NET_STATE_CRITICAL_SECTION))
#define NET_UNLOCK_CRITICAL(ctx)					((ctx->state &= ~(IF_NET_STATE_BUSY | IF_NET_STATE_CRITICAL_SECTION)))

typedef struct net_context * net_context_p;
typedef struct net_context net_context;
typedef struct net_ssid net_ssid;
typedef struct net_ssid * net_ssid_p;
typedef struct net_ssidrec net_ssidrec;
typedef struct net_ssidrec * net_ssidrec_p;
typedef struct net_btrec net_btrec;
typedef struct net_btrec * net_btrec_p;
typedef struct net_conn net_conn;
typedef struct net_conn * net_conn_p;
typedef struct net_status net_status;
typedef struct net_status * net_status_p;
typedef struct net_buffer * net_buffer_p;

typedef union net_addr{
    struct {
      uint8 b4,b3,b2,b1;
    } b_addr;
    struct {
      uint16 w2,w1;
    } w_addr;
    uint32 dw_addr;
} net_addr;

//net message parameters
typedef struct net_async_params {
	uint8 type;
	void * param1;
	void * param2;
	void * param3;
	void * param4;
	void * param5;
	void * param6;
	void * param7;
} net_async_params;

typedef struct net_buffer {
	uint8 mode;
	uint8 * buffer;
	uint16 bufsz;
	uint16 buflen;
	uint16 bufend;
	void (*buf_release)(void * buf);
	//listen callback
	void * ctx;			//custom context
	void (* listen_callback)(void * ctx, net_buffer_p buffer);
} net_buffer;

struct net_conn {
	net_buffer base;
	uint8 id;
	uint8 state;
	net_context * netctx;
	//connection buffer
	uint32 timeout;
	uint16 port;
	net_addr remote_ip;
	uint16 remote_port;
} ;

struct net_status {
	uint8 channel;
	uint8 remote_ip[4];
	uint16 remote_port;
	uint16 local_port;
} ;

struct net_context {
	UART_HandleTypeDef handle;
	uint32 baudrate;
	GPIO_TypeDef * port;
	uint16 rst;
	uint16 cs;
	uint16 state;						//current device state (connected, sleep, busy, etc)
	int16 att;							//signal attenuation
	uint8 type;							//interface type
	uint8 name[SHARD_MAX_NODE_NAME];	//interface name
	uint8 ipv4[4];						
	uint8 mac[6];
	uint8 dns[4];
	uint16 con_id;				//current socket identifier, only for RTL8710 (added 2017.05.24)
	//static configuration
	uint8 staip[4];
	uint8 stadns1[4];
	uint8 stadns2[4];
	uint8 fcnctr;						//force connect counter
	net_ssid_p ssid_list;				//list of ssid for wifi connection
	void * nb_ctx;						//netbios context
	//device APIs
	void (* reset)(net_context_p ctx);
	uint8 (* init)(net_context_p ctx);
	uint8 (* try_connect)(net_context_p);
	uint8 (* list)(net_context_p ctx);
	uint8 (* join)(net_context_p ctx, uint8 * ssidname, uint8 * username, uint8 * password);
	uint8 (* sleep)(net_context_p ctx);
	uint8 (* get_status)(net_context_p ctx, net_status_p stat);
	uint8 (* get_config)(net_context_p ctx);
	uint8 (* power_down)(net_context_p ctx);
	uint8 (* power_up)(net_context_p ctx);
	uint8 (* disable)(net_context_p ctx);		//deactivate module, back to initial state
	uint8 (* enable)(net_context_p ctx);		//activate module, set to running state	
	
	//socket APIs
	net_conn_p (* sock_open)(net_context_p ctx, uint8 * host, uint16 port, uint16 mode);
	uint16 (* sock_send)(net_conn_p conn, uint8 * payload, uint16 length);
	uint16 (* sock_accept)(net_conn_p conn);
	uint16 (* sock_recv)(net_conn_p conn, uint8 * response, uint16 bufsz);
	void (* sock_close)(net_conn_p conn);
	//transport APIs
	uint16 (* send_tcp)(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response);
	uint16 (* send_udp)(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response);
	uint16 (* send_ssl)(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response);
	//interface callback (OS related)
	void * p_ctx;				//prepare context
	void * f_ctx;				//finish context
	uint8 cb_depth;				//callback depth, prevent nested prepare/finish callback
	void (* prepare_callback)(void * ctx);
	void (* finish_callback)(void * ctx);
	void (* exception_handler)(net_context_p ctx, const char * text);
};

struct net_ssid {
	net_ssid_p next;
	net_context_p netctx;
	uint8 state;
	int16 att;						//line attenuation
	uint8 mac[6];				//mac address
	uint8 sec;					//security mode
	uint8 name[IF_NET_MAX_SSID_NAME];
};

struct net_ssidrec {
	uint8 magic;
	uint8 ordinal;
	uint8 name[30];
	uint8 password[32];
	uint8 username[32];
};

struct net_btrec {
	uint8 magic;
	uint8 mac[6];
	uint8 password[32];
};

uint8 if_net_init(net_context_p ctx);
void if_net_init_config(net_context_p ctx);
uint8 if_net_ssid_list(net_context_p ctx);
uint8 if_net_ssid_join(net_context_p ctx, uint8 * ssidname, uint8 * username, uint8 * password);
void if_net_ssid_clear(net_ssid_p list);
uint8 if_net_try_connect(net_context_p ctx);
uint8 if_net_is_connected(net_context_p ctx, uint8 * ssidname);
void if_net_reset(net_context_p ctx);
void if_net_deep_sleep(net_context_p ctx);
uint8 if_net_get_ipconfig(net_context_p ctx);
uint8 if_net_get_status(net_context_p ctx, net_status_p stat);

//net generic tcp APIs
net_conn_p if_net_tcp_open(net_context_p ctx, uint8 * host, uint16 port, uint16 mode);
uint16 if_net_tcp_send(net_conn_p ctx, uint8 * payload, uint16 length);
uint16 if_net_tcp_accept(net_conn_p ctx);
uint16 if_net_tcp_recv(net_conn_p ctx, uint8 * response, uint16 bufsz);
void if_net_tcp_close(net_conn_p ctx); 
uint16 if_net_tcp_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response);

//net generic udp APIs
net_conn_p if_net_udp_open(net_context_p ctx, uint8 * host, uint16 port, uint16 mode);
uint16 if_net_udp_send(net_conn_p ctx, uint8 * payload, uint16 length, uint8 addr[], uint16 port);
uint16 if_net_udp_accept(net_conn_p conn);
uint16 if_net_udp_recv(net_conn_p ctx, uint8 * response, uint16 bufsz);
void if_net_udp_close(net_conn_p conn);
uint16 if_net_udp_transmit(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response) ;

//generic conn APIs
uint8 if_net_exec_listener(net_context_p ctx);		//must be called by external thread
void if_net_set_listen_callback(net_conn_p conn, void * ctx, void (* callback)(void *, net_buffer_p));
void if_net_set_buffer(net_conn_p conn, uint8 * buffer, uint16 bufsz);
void if_net_set_timeout(net_conn_p conn, uint16 timeout);

//power saving APIs
void if_net_wake(net_context_p ctx);	
void if_net_sleep(net_context_p ctx);
void if_net_power_up(net_context_p ctx);
uint8 if_net_power_down(net_context_p ctx);
uint8 if_net_disable(net_context_p ctx);
uint8 if_net_enable(net_context_p ctx);

//ssid record APIs
uint8 if_net_ssidrec_read(void * handle, uint8 index, net_ssidrec * record);
void if_net_ssidrec_push(void * handle, uint8 * name, uint8 * username, uint8 * password);
uint8 if_net_ssidrec_count(void * handle) ;

//net prepare events
void if_net_set_prepare_context(net_context_p ctx, void * cb_ctx);
void if_net_set_prepare_callback(net_context_p ctx, void (* prepare)(void *));
void if_net_set_finish_context(net_context_p ctx, void * cb_ctx);
void if_net_set_finish_callback(net_context_p ctx, void (* finish)(void *));
void if_net_set_exception_callback(net_context_p ctx, void  (* exception)(net_context_p, const char * ));

//net buffer APIs
void net_buffer_reset(void);
uint8 net_buffer_dequeue(uint8 * c);
uint16 net_buffer_length(void);
uint16 net_buffer_tail(void);

//net conn APIs
net_conn * net_conn_get_client(uint16 index);
net_conn * net_conn_get_default(void);
void net_conn_set_default(net_conn * conn);
net_conn * net_conn_create(net_context_p ctx, uint16 mode, uint32 timeout, uint16 port, uint8 * buffer, uint16 bufsz, void (* listen_callback)(void *, net_conn *));
void net_conn_close(net_conn * conn);

//available drivers
uint8 esp_init(net_context_p ctx);
uint8 simcom_init(net_context_p ctx) ;
#endif
