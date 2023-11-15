#ifndef SMTP__H
#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\..\interfaces\inc\if_apis.h"

#define TIME_IN_SEC											10		// how long client will wait for server response in non-blocking mode
#define BUFFER_SIZE 										2048	  // SendData and RecvData buffers sizes
#define MSG_SIZE_IN_MB 									1		// the maximum size of the message with all attachments
#define COUNTER_VALUE									100		// how many times program will try to receive data


#define SMTP_XPRIORITY_HIGH 							2
#define SMTP_XPRIORITY_NORMAL						3
#define SMTP_XPRIORITY_LOW  							4

#define SMTP_CSMTP_NO_ERROR 						0
#define SMTP_WSA_STARTUP								100 // WSAGetLastError()
#define SMTP_WSA_VER										0x10
#define SMTP_WSA_SEND									0x11
#define SMTP_WSA_RECV									0x12
#define SMTP_WSA_CONNECT								0x14
#define SMTP_WSA_GETHOSTBY_NAME_ADDR		0x15
#define SMTP_WSA_INVALID_SOCKET					0x16
#define SMTP_WSA_HOSTNAME							0x17
#define SMTP_WSA_IOCTLSOCKET						0x18
#define SMTP_WSA_SELECT								0x19
#define SMTP_BAD_IPV4_ADDR							0x40
#define SMTP_UNDEF_MSG_HEADER						200
#define SMTP_UNDEF_MAIL_FROM						201
#define SMTP_UNDEF_SUBJECT							202
#define SMTP_UNDEF_RECIPIENTS						203
#define SMTP_UNDEF_LOGIN								204
#define SMTP_UNDEF_PASSWORD						205
#define SMTP_UNDEF_RECIPIENT_MAIL					206
#define SMTP_COMMAND_MAIL_FROM					300
#define SMTP_COMMAND_EHLO							301
#define SMTP_COMMAND_AUTH_LOGIN				302
#define SMTP_COMMAND_DATA							303
#define SMTP_COMMAND_QUIT							304
#define SMTP_COMMAND_RCPT_TO						305
#define SMTP_MSG_BODY_ERROR						306
#define SMTP_CONNECTION_CLOSED					400 // by server
#define SMTP_SERVER_NOT_READY						401 // remote server
#define SMTP_SERVER_NOT_RESPONDING				402	
#define SMTP_SELECT_TIMEOUT							403
#define SMTP_FILE_NOT_EXIST							404
#define SMTP_MSG_TOO_BIG								405
#define SMTP_BAD_LOGIN_PASS							406
#define SMTP_UNDEF_XYZ_RESPONSE					407
#define SMTP_LACK_OF_MEMORY						408
#define SMTP_TIME_ERROR								409
#define SMTP_RECVBUF_IS_EMPTY						410
#define SMTP_SENDBUF_IS_EMPTY						411
#define SMTP_OUT_OF_MSG_RANGE						412

typedef struct smtp_context {
	net_context_p netctx;
	uint8 username[64];
	uint8 password[64];
	uint8 server[64];
	uint8 priority;
	uint16 port;
} smtp_context;

typedef smtp_context * smtp_context_p;

//SMTP APIs
uint8 smtp_init(net_context_p ctx, uint8 * server, uint16 port, uint8 * username, uint8 * password, smtp_context_p smtp);
uint8 smtp_send(smtp_context_p ctx, uint8 * from, uint8 * subject, uint8 * to, uint8 * msg);
void smtp_format_header(smtp_context_p ctx, uint8 * header, uint8 * from, uint8 * subject, uint8 * to);

#define SMTP__H
#endif