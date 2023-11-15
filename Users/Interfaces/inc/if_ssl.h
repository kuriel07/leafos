#include "..\..\defs.h"
#include "..\..\config.h"
#include "if_apis.h"
#include "wolfssl/ssl.h"
#include "cert_data.h"

#ifndef _IF_SSL__H
#define _IF_SSL__H
typedef struct ssl_handle * ssl_handle_p;
typedef struct ssl_cert * ssl_cert_p;
typedef struct ssl_keys * ssl_keys_p;

typedef struct ssl_cert {
	uint32 magic;
	uint16 length;
	uint8 bytes[1];
} ssl_cert;

typedef struct ssl_keys {
	uint32 magic;
	uint16 length;
	uint8 bytes[1];
} ssl_keys;

typedef struct ssl_handle {
	net_buffer base;
	uint16 rd_index;
	uint16 rd_size;
	net_conn_p conn;
	void * ssl;
	void * ssl_ctx;
	ssl_cert_p cert;
	ssl_keys_p keys;
} ssl_handle;

//one time connection
//uint16 if_net_ssl_init(net_context_p nctx, uint8 * certbuf, uint16 ca_length) ;
//void if_net_ssl_release(net_context_p nctx);
void if_ssl_set_listen_callback(ssl_handle_p ssl, void * ctx, void (* callback)(void *, net_buffer_p));
ssl_cert_p if_ssl_create_cert(uint8 * cert, uint16 size);
void if_ssl_release_cert(ssl_cert_p cert);
ssl_keys_p if_ssl_create_keys(uint8 * keys, uint16 size);
void if_ssl_release_keys(ssl_keys_p keys);

//unvarnished connection
ssl_handle_p if_net_ssl_open(net_context_p ctx, uint8 * host, uint16 port, uint16 mode, ssl_cert_p cert, ssl_keys_p keys) ;
void if_net_ssl_close(ssl_handle_p handle);
uint16 if_net_ssl_send(ssl_handle_p ssdl, uint8 * payload, uint16 length) ;
uint16 if_net_ssl_recv(ssl_handle_p ssdl, uint8 * response, uint16 bufsize);
uint16 if_net_ssl_transmit(net_context_p  nctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response, ssl_cert_p cert);
uint16 if_ssl_fetch(ssl_handle_p handle, uint8 * payload, uint16 length, uint8 * response) ; 

//async APIs
ssl_handle_p if_ssl_open_async (net_context_p ctx, uint8 * host, uint16 port, uint16 mode, ssl_cert_p cert, ssl_keys_p keys);
uint16 if_ssl_send_async (ssl_handle_p ctx, uint8 * payload, uint16 length);
uint16 if_ssl_recv_async (ssl_handle_p ctx, uint8 * response, uint16 bufsize);
void if_ssl_close_async (ssl_handle_p ctx) ;
uint16 if_ssl_transmit_async(net_context_p ctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response, ssl_cert_p cert);
void * if_ssl_init_async(net_context_p nctx, uint16 mode, ssl_cert_p cert, ssl_keys_p keys);
void if_ssl_release_async(void * ssl_ctx);


#define _IF_SSL__H
#endif

