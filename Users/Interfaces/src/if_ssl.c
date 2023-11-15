#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include <string.h>
#if SHARD_SSL_SUPPORT
#include "wolfssl\ssl.h"
//implementation of wolfSSL for tcp communication (2016.06.08)

extern uint8 gba_net_buffer[NET_BUFFER_SIZE];

static int if_ssl_send_raw(WOLFSSL * wctx, char * buf, int sz, void * ictx) {
	int len;
	if_delay(200);		//wait from previous processing
	ssl_handle_p ctx = (ssl_handle_p)ictx;
	net_context_p netctx = ctx->conn->netctx;
	return netctx->sock_send(((ssl_handle_p)ictx)->conn, (uint8 *)buf, sz);// == 0) return sz;			//amount of bytes sent
}

static int if_ssl_recv_raw(WOLFSSL * wctx, char * buf, int sz, void * ictx) {
	int32 len;
	ssl_handle_p ctx = (ssl_handle_p)ictx;
	net_context_p netctx = ctx->conn->netctx;
	if(ctx->rd_index == ctx->rd_size) {
		//if(ctx->rd_size != 0) return sz;						//data already completed in buffer
		ctx->rd_index = ctx->rd_size = 0;
		if(if_net_tcp_accept(ctx->conn) != 0) return -1;		//timeout
		//ctx->rd_size = ctx->base.buflen;
		len = (int16)netctx->sock_recv(ctx->conn, (uint8 *)ctx->base.buffer, ctx->conn->base.buflen);
		if(len < 0) return len;
		//if(len < sz) sz = len;
		memcpy(buf, ctx->base.buffer + ctx->rd_index, sz);
		ctx->rd_index += sz;		//data readed
		ctx->rd_size += len;		//actual size
	} else if(ctx->rd_index < ctx->rd_size) {
		len = (ctx->rd_size - ctx->rd_index);
		if(len < sz) sz = len;
		memcpy(buf, ctx->base.buffer + ctx->rd_index, sz);
		ctx->rd_index += sz;
	}
	return sz;
}
ssl_cert_p if_ssl_create_cert(uint8 * bytes, uint16 size) {
	ssl_cert_p cert = (ssl_cert_p)os_alloc(sizeof(ssl_cert) + size);
	if(cert == NULL) return NULL;
	cert->magic = 0xCECC92EA;
	cert->length = size;
	memcpy(cert->bytes, bytes, size);
	return cert;
}

void if_ssl_release_cert(ssl_cert_p cert) {
	if(cert == NULL) return;
	if(cert->magic == 0xCECC92EA) {
		memset(cert, 0, sizeof(ssl_cert));
		os_free(cert);
	}
}

ssl_keys_p if_ssl_create_keys(uint8 * keys, uint16 size) {
	ssl_keys_p ckey = (ssl_keys_p)os_alloc(sizeof(ssl_keys) + size);
	if(keys == NULL) return NULL;
	ckey->magic = 0xAEAA92EC;
	ckey->length = size;
	memcpy(ckey->bytes, keys, size);
	return ckey;
}

void if_ssl_release_keys(ssl_keys_p keys) {
	if(keys == NULL) return;
	if(keys->magic == 0xAEAA92EC) {
		memset(keys, 0, sizeof(ssl_keys));
		os_free(keys);
	}
}

static void if_ssl_listen_callback(void * ctx, net_buffer_p netbuf) {
	ssl_handle_p ssl = (ssl_handle_p)ctx;
	//start decoding data to the buffer
	ssl->base.buflen = if_ssl_recv_async(ssl, ssl->base.buffer, ssl->base.bufsz);
	if(ssl->base.listen_callback != NULL) 
		ssl->base.listen_callback(ssl->base.ctx, (net_buffer_p)ssl);
}

void if_ssl_set_listen_callback(ssl_handle_p ssl, void * ctx, void (* callback)(void *, net_buffer_p)) {
	if(ssl == NULL) return;
	ssl->base.ctx = ctx;
	ssl->base.listen_callback = callback;
	if_net_set_listen_callback(ssl->conn, ssl, if_ssl_listen_callback);
}

ssl_handle_p if_ssl_open_async(net_context_p ctx, uint8 * host, uint16 port, uint16 mode, ssl_cert_p cert, ssl_keys_p keys) {
	ssl_handle_p ret = NULL;
	int err =0, rlen;
    WOLFSSL * ssl = 0;
	void * ssl_ctx;
	uint8 * buffer;
	uint8 err_buffer[80];
	net_conn_p conn;
	uint16 tcp_mode = NET_TYPE_TCP | (mode & NET_LISTEN);
	//if(cert == NULL) return NULL;
	if((ctx->state & IF_NET_STATE_CONNECTED) == 0) return NULL;		//not connected
	//if((mode & NET_LISTEN) == 0) NET_LOCK(ctx);
	ssl_ctx = if_ssl_init_async(ctx, mode, cert, keys);
	if(ssl_ctx == NULL) goto exit_ssl_open;
	ssl = wolfSSL_new(ssl_ctx);
    if((conn = ctx->sock_open(ctx, host, port, tcp_mode)) == NULL) {
		exit_ssl_open:
		if_ssl_release_async(ssl_ctx);
		//if((mode & NET_LISTEN) == 0) NET_UNLOCK(ctx);
		return NULL;
	}
	ret = os_alloc(sizeof(ssl_handle) + conn->base.bufsz);
	if(ret == NULL) {
		wolfSSL_free(ssl);
		if_ssl_release_async(ssl_ctx);
		os_free(ret);
		return NULL;
	}
	buffer = (uint8 *)ret + sizeof(ssl_handle);
	memset(ret, 0, sizeof(ssl_handle));
	ret->conn = conn;
	ret->ssl = ssl;
	ret->ssl_ctx = ssl_ctx;
	//ret->rd_buffer = gba_net_buffer + 2048;
	ret->base.mode = (mode | NET_TYPE_SSL);
	ret->base.buffer = (uint8 *)ret + sizeof(ssl_handle);
	ret->base.bufsz = conn->base.bufsz;
	ret->base.buflen = 0;
	ret->rd_index = 0;
	ret->rd_size = 0;
	ret->cert = cert;
	ret->keys = keys;
        
    wolfSSL_set_fd(ret->ssl, (int)ctx);			//set current net context
	//custom context
	wolfSSL_SetIOReadCtx(ret->ssl, ret);
	wolfSSL_SetIOWriteCtx(ret->ssl, ret);
	//wolfSSL_use_certificate_buffer(ssl,  (const uint8 *)certbuf, ca_length, SSL_FILETYPE_ASN1);
	if((mode & NET_LISTEN) == 0) {
		if (wolfSSL_connect(ret->ssl) != SSL_SUCCESS) {
			ctx->sock_close(ret->conn);
			err = wolfSSL_get_error(ret->ssl, 0);
			wolfSSL_ERR_error_string_n(err, (char *)err_buffer, 80);
			if_set_last_error(err, err_buffer, strlen((const char *)err_buffer)) ;
			wolfSSL_free(ret->ssl);
			free(ret);
			return NULL;
		}
	}
	return ret;
}

void * if_ssl_init_async(net_context_p nctx, uint16 mode, ssl_cert_p cert, ssl_keys_p keys) {
	void * ssl_ctx;
	WOLFSSL_METHOD * (* method)(void) = wolfSSLv23_client_method;
	wolfSSL_Init();
	//if((nctx->ssl_ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) == NULL) {
	if(mode & NET_LISTEN) {
		method = wolfSSLv23_server_method;
	}
	if((ssl_ctx = wolfSSL_CTX_new(method())) == NULL) {
		wolfSSL_CTX_free(ssl_ctx);
		wolfSSL_Cleanup();
		return NULL;
	}
	if(wolfSSL_CTX_load_verify_buffer(ssl_ctx, (const uint8 *)server_cacert_der_1024, sizeof(server_cacert_der_1024), SSL_FILETYPE_ASN1) != SSL_SUCCESS) {
		wolfSSL_CTX_free(ssl_ctx);
		wolfSSL_Cleanup();
		return NULL;
	}
	if((mode & NET_LISTEN) && cert != NULL && wolfSSL_CTX_use_certificate_buffer(ssl_ctx, (const uint8 *)cert->bytes, cert->length, SSL_FILETYPE_ASN1) != SSL_SUCCESS) {
		wolfSSL_CTX_free(ssl_ctx);
		wolfSSL_Cleanup();
		return NULL;
	}
	if((mode & NET_LISTEN) && keys != NULL && wolfSSL_CTX_use_PrivateKey_buffer(ssl_ctx, (const uint8 *)keys->bytes, keys->length, SSL_FILETYPE_ASN1) != SSL_SUCCESS) {
		wolfSSL_CTX_free(ssl_ctx);
		wolfSSL_Cleanup();
		return NULL;
	}
	wolfSSL_SetIORecv(ssl_ctx, if_ssl_recv_raw);
	wolfSSL_SetIOSend(ssl_ctx, if_ssl_send_raw);
	//wolfSSL_CTX_UseTruncatedHMAC(nctx->ssl_ctx);
	wolfSSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, 0);
	return ssl_ctx;
}

void if_ssl_release_async(void * ssl_ctx) {
	wolfSSL_CTX_free(ssl_ctx);
	wolfSSL_Cleanup();
}


uint16 if_ssl_send_async(ssl_handle_p ssdl, uint8 * payload, uint16 length) {
	int err =0, rlen;
	uint8 err_buffer[80];
	if (wolfSSL_write(ssdl->ssl, payload, length) != length) {
		err = wolfSSL_get_error(ssdl->ssl, 0);
		wolfSSL_ERR_error_string_n(err, (char *)err_buffer, 80);
		if_set_last_error(err, err_buffer, strlen((const char *)err_buffer)) ;
	}
	return length;
}

uint16 if_ssl_recv_async(ssl_handle_p ssdl, uint8 * response, uint16 bufsize) {
	uint16 recvLen = 0;
	int err =0, rlen;
	uint8 err_buffer[80];
	recvLen = 0;
	uint8 * temp_buffer = (gba_net_buffer + 16384);
	while((rlen = wolfSSL_read(ssdl->ssl, temp_buffer + recvLen, NET_BUFFER_SIZE - recvLen)) != 0) {
		if(rlen == (int32)-1) break;		//error
		recvLen += rlen;
	}
	//copy temporary buffer to response buffer
	memcpy(response, temp_buffer, recvLen);
	return recvLen;
}

void if_ssl_close_async(ssl_handle_p handle) {
	net_context_p netctx = handle->conn->netctx;
    wolfSSL_shutdown(handle->ssl);			//shutdown wolf ssl
	netctx->sock_close(handle->conn);			//close tcp connection
    wolfSSL_free(handle->ssl);					//free ssl context
	if(handle->ssl_ctx != NULL) {
		if_ssl_release_async(handle->ssl_ctx);
	}
	if(handle->cert != NULL) if_ssl_release_cert(handle->cert);
	if(handle->keys != NULL) if_ssl_release_keys(handle->keys);
	handle->cert = NULL;
	handle->keys = NULL;
	os_free(handle);
}

uint16 if_ssl_fetch(ssl_handle_p handle, uint8 * payload, uint16 length, uint8 * response) {
	uint8 err_buffer[80];
	uint16 recvLen = 0;
	int err =0, rlen;
	uint8 * temp_buffer = (gba_net_buffer + 16384);
	//only send one record request
    if (wolfSSL_write(handle->ssl, payload, length) != length) {
		err = wolfSSL_get_error(handle->ssl, 0);
		wolfSSL_ERR_error_string_n(err, (char *)err_buffer, 80);
		if_set_last_error(err, err_buffer, strlen((const char *)err_buffer)) ;
		goto exit_transmit;
	}
	
	//read all response onto temporary buffer
	recvLen = 0;
	while((rlen = wolfSSL_read(handle->ssl, temp_buffer + recvLen, NET_BUFFER_SIZE - recvLen)) != 0) {
		recvLen += rlen;
	}
	exit_transmit:
	//copy temporary buffer to response buffer
	memcpy(response, temp_buffer, recvLen);
	return recvLen;
}

uint16 if_ssl_transmit_async(net_context_p nctx, uint8 * host, uint16 port, uint8 * payload, uint16 length, uint8 * response, ssl_cert_p cert) {
    WOLFSSL*        ssl    = 0;
	ssl_handle * ssdl;
	int err =0, rlen;
	uint8 err_buffer[80];
	uint16 recvLen = 0;
	uint8 * temp_buffer = (gba_net_buffer + 16384);
	//ssl_handle_p ctx = (ssl_handle_p)ictx;
	//net_context_p netctx = ctx->conn->netctx;
	//if(nctx->ssl_ctx == NULL) return 0;
	//try to connect in-case wakeup from sleep
	if((nctx->state & IF_NET_STATE_CONNECTED) == 0) {		//not connected
		//if_net_try_connect_async(nctx);
		nctx->try_connect(nctx);
	}
	NET_LOCK(nctx);
    if((ssdl = if_ssl_open_async(nctx, host, port, NET_TRANSMIT, NULL, NULL)) == NULL) return 0;
	//only send one record request
    if (wolfSSL_write(ssdl->ssl, payload, length) != length) {
		err = wolfSSL_get_error(ssdl->ssl, 0);
		wolfSSL_ERR_error_string_n(err, (char *)err_buffer, 80);
		if_set_last_error(err, err_buffer, strlen((const char *)err_buffer)) ;
		goto exit_transmit;
	}
	
	//read all response onto temporary buffer
	recvLen = 0;
	while((rlen = wolfSSL_read(ssdl->ssl, temp_buffer + recvLen, NET_BUFFER_SIZE - recvLen)) != 0) {
		if(rlen == (int32)-1) break;		//error
		recvLen += rlen;
	}
	exit_transmit:
	//shutdown still require the use of net_buffer, do not copy the result of operation until all operation completed
	if_ssl_close_async(ssdl);					//close connection
	NET_UNLOCK(nctx);
	//copy temporary buffer to response buffer
	memcpy(response, temp_buffer, recvLen);
	return recvLen;
}

#endif
