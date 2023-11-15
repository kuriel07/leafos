////////////////////////////////////////////////////////////////////////////////
// Original class CFastSmtp written by 
// christopher w. backen <immortal@cox.net>
// More details at: http://www.codeproject.com/KB/IP/zsmtp.aspx
// 
// Modifications introduced by Jakub Piwowarczyk:
// 1. name of the class and functions
// 2. new functions added: SendData,ReceiveData and more
// 3. authentication added
// 4. attachments added
// 5 .comments added
// 6. DELAY_IN_MS removed (no delay during sending the message)
// 7. non-blocking mode
// More details at: http://www.codeproject.com/KB/mcpp/CSmtp.aspx
////////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\..\defs.h"
#include "..\..\config.h"
#include "..\..\interfaces\inc\if_apis.h"
#include "..\..\toolkit\inc\tk_apis.h"
#include "..\inc\smtp.h"

uint8_c BOUNDARY_TEXT[] = "__MESSAGE__ID__54yg6f6h6y456345";


////////////////////////////////////////////////////////////////////////////////
//        NAME: SmtpXYZdigits
// DESCRIPTION: Converts three letters from RecvBuf to the number.
//   ARGUMENTS: none
// USES GLOBAL: RecvBuf
// MODIFIES GL: none
//     RETURNS: integer number
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
int smtp_convert_digits(uint8 * rcv)
{
	if(rcv == NULL)
		return 0;
	return (rcv[0]-'0')*100 + (rcv[1]-'0')*10 + rcv[2]-'0';
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: Send
// DESCRIPTION: Sending the mail. .
//   ARGUMENTS: none
// USES GLOBAL: m_sSMTPSrvName, m_iSMTPSrvPort, SendBuf, RecvBuf, m_sLogin,
//              m_sPassword, m_sMailFrom, Recipients, CCRecipients,
//              BCCRecipients, m_sMsgBody, Attachments, 
// MODIFIES GL: SendBuf 
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-08
////////////////////////////////////////////////////////////////////////////////
uint8 smtp_init(net_context_p ctx, uint8 * server, uint16 port, uint8 * username, uint8 * password, smtp_context_p smtp) {
	if(ctx == NULL) return -1;
	if(server == NULL) return -1;
	if(smtp == NULL) return -1;
	smtp->netctx = ctx;
	smtp->username[0] = 0;
	smtp->password[0] = 0;
	if(server != NULL) strncpy((char *)smtp->server, (char *)server, 64);
	if(username != NULL) strncpy((char *)smtp->username, (char *)username, 64);
	if(password != NULL) strncpy((char *)smtp->password, (char *)password, 64);
	smtp->port = port;
	smtp->priority = SMTP_XPRIORITY_NORMAL;
	return 0;
}

void smtp_delete(smtp_context_p smtp) {
	os_free(smtp);
}

uint8 smtp_send(smtp_context_p ctx, uint8 * from, uint8 * subject, uint8 * to, uint8 * msg)
{
	uint8 bAccepted = FALSE;
	uint16 len;
	uint8 buffer[1024];
	uint8 hbuffer[128];
	uint32 timeout = 12000;
	uint8 ret = -1;
	net_conn_p conn;
	//uint8 encoded_login[128];

	// ***** CONNECTING TO SMTP SERVER *****

	// connecting to remote host:
	//if( (hSocket = ConnectRemoteServer(m_sSMTPSrvName.c_str(), m_iSMTPSrvPort)) == INVALID_SOCKET ) 
	//	throw ECSmtp(ECSmtp::WSA_INVALID_SOCKET);
	if((conn = if_net_tcp_open(ctx->netctx, ctx->server, ctx->port, NET_TRANSMIT)) == NULL) goto exit_mail_send;

	bAccepted = FALSE;
	//ReceiveData();
	if(if_net_tcp_accept(conn) == 0) {
		len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
		switch(smtp_convert_digits(buffer))
		{
			case 220:
				bAccepted = FALSE;
				break;
			default:
				ret = SMTP_SERVER_NOT_READY;
				goto exit_mail_send;
		}
	} else  goto exit_mail_send;

	// EHLO <SP> <domain> <CRLF>
	sprintf(buffer,"EHLO %s\r\n","domain");
	//SendData();
	if_net_tcp_send(conn, buffer, strlen(buffer));
	bAccepted = FALSE;
	if(if_net_tcp_accept(conn) == 0) {
		len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
		switch(smtp_convert_digits(buffer))
		{
			case 250:
				bAccepted = TRUE;
				break;
			default: 
				ret = SMTP_COMMAND_EHLO;
				goto exit_mail_send;
		}
	} else  goto exit_mail_send;

	// AUTH <SP> LOGIN <CRLF>
	if(strlen(ctx->username) != 0) {
		strcpy((char *)buffer,"AUTH LOGIN\r\n");
		//SendData();
		if_net_tcp_send(conn, buffer, strlen((const char *)buffer));
		bAccepted = FALSE;
		if(if_net_tcp_accept(conn) == 0) {
			len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
			switch(smtp_convert_digits(buffer))
			{
				case 250:
					break;
				case 334:
					bAccepted = FALSE;
					break;
				default:
					ret = SMTP_COMMAND_AUTH_LOGIN;
					goto exit_mail_send;
			}
		} else  goto exit_mail_send;

		tk_base64_encode(ctx->username, strlen(ctx->username), hbuffer);
		sprintf(buffer, "%s\r\n", hbuffer);
		//SendData();
		if_net_tcp_send(conn, buffer, strlen(buffer));
		bAccepted = FALSE;
		if(if_net_tcp_accept(conn) == 0) {
			len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
			switch(smtp_convert_digits(buffer))
			{
				case 334:
					bAccepted = FALSE;
					break;
				default:
					ret = SMTP_UNDEF_XYZ_RESPONSE;
					goto exit_mail_send;
			}
		} else  goto exit_mail_send;
		
		// send password:
		//if(!m_sPassword.size())
		//	throw ECSmtp(ECSmtp::UNDEF_PASSWORD);
		//std::string encoded_password = base64_encode(reinterpret_cast<const unsigned char*>(m_sPassword.c_str()),m_sPassword.size());
		tk_base64_encode(ctx->password, strlen(ctx->password), hbuffer);
		sprintf(buffer,"%s\r\n",hbuffer);
		//SendData();
		if_net_tcp_send(conn, buffer, strlen(buffer));
		bAccepted = FALSE;
		if(if_net_tcp_accept(conn) == 0) {
			len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
			switch(smtp_convert_digits(buffer))
			{
				case 235:
					bAccepted = TRUE;
					break;
				case 334:
					break;
				case 535:
					ret = SMTP_BAD_LOGIN_PASS;
					goto exit_mail_send;
				default:
					ret = SMTP_UNDEF_XYZ_RESPONSE;
					goto exit_mail_send;
			}
		}
	}

	// ***** SENDING E-MAIL *****
	
	// MAIL <SP> FROM:<reverse-path> <CRLF>
	//if(!m_sMailFrom.size())
	//	throw ECSmtp(ECSmtp::UNDEF_MAIL_FROM);
	sprintf(buffer,"MAIL FROM:<%s>\r\n",from);
	//SendData();
	if_net_tcp_send(conn, buffer, strlen(buffer));
	bAccepted = FALSE;
	if(if_net_tcp_accept(conn) == 0) {
		len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
		switch(smtp_convert_digits(buffer))
		{
			case 250:
				bAccepted = TRUE;
				break;
			default:
				ret = SMTP_COMMAND_MAIL_FROM;
				goto exit_mail_send;
		}
	} else  goto exit_mail_send;

	// RCPT <SP> TO:<forward-path> <CRLF>
	//if(!(rcpt_count = Recipients.size()))
	//	throw ECSmtp(ECSmtp::UNDEF_RECIPIENTS);
	//for(i=0;i<Recipients.size();i++)
	//{
	sprintf(buffer,"RCPT TO:<%s>\r\n", to);
	//SendData();
	if_net_tcp_send(conn, buffer, strlen(buffer));
	bAccepted = FALSE;
	if(if_net_tcp_accept(conn) == 0) {
		len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
		switch(smtp_convert_digits(buffer))
		{
			case 250:
				bAccepted = TRUE;
				break;
			default:
				//rcpt_count--;
				break;
		}
	} else  goto exit_mail_send;
	// DATA <CRLF>
	strcpy(buffer,"DATA\r\n");
	//SendData();
	if_net_tcp_send(conn, buffer, strlen(buffer));
	bAccepted = FALSE;
	if(if_net_tcp_accept(conn) == 0) {
		len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
		switch(smtp_convert_digits(buffer))
		{
			case 354:
				bAccepted = TRUE;
				break;
			case 250:
				break;
			default:
				ret = SMTP_COMMAND_DATA;
				goto exit_mail_send;
		}
	} else  goto exit_mail_send;
	
	// send header(s)
	//smtp_format_header(buffer);
	smtp_format_header(ctx, buffer, from, subject, to);
	//SendData();
	if_net_tcp_send(conn, buffer, strlen(buffer));
	if_delay(2000);
	// send text message
	if_net_tcp_send(conn, msg, strlen(msg));
	if_delay(2000);
	// <CRLF> . <CRLF>
	strcpy(buffer,"\r\n.\r\n");
	if_net_tcp_send(conn, buffer, strlen(buffer));
	bAccepted = FALSE;
	if(if_net_tcp_accept(conn) == 0) {
		len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
		switch(smtp_convert_digits(buffer))
		{
			case 221:			//automatically closed by server
				bAccepted = TRUE;
				ret = 0;
				goto exit_mail_send;
			case 250:
				bAccepted = TRUE;
				break;
			default:
				ret = SMTP_MSG_BODY_ERROR;
				goto exit_mail_send;
		}
	} else  goto exit_mail_send;

	// ***** CLOSING CONNECTION *****
	
	// QUIT <CRLF>
	strcpy(buffer,"QUIT\r\n");
	//SendData();
	if_net_tcp_send(conn, buffer, strlen(buffer));
	bAccepted = FALSE;
	if(if_net_tcp_accept(conn) == 0) {
		len = if_net_tcp_recv(conn, buffer, sizeof(buffer));
		switch(smtp_convert_digits(buffer))
		{
			case 221:
				bAccepted = TRUE;
				break;
			default:
				ret = SMTP_COMMAND_QUIT;
				goto exit_mail_send;
		}
	} else  goto exit_mail_send;
	
	exit_mail_send:
	if_net_tcp_close(conn);
	
	return 0;		//success
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: FormatHeader
// DESCRIPTION: Prepares a header of the message.
//   ARGUMENTS: char* header - formated header string
// USES GLOBAL: Recipients, CCRecipients, BCCRecipients
// MODIFIES GL: none
//     RETURNS: void
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
//							JP 2010-07-07
////////////////////////////////////////////////////////////////////////////////
void smtp_format_header(smtp_context_p ctx, uint8 * header, uint8 * from, uint8 * subject, uint8 * to)
{
	char month[][4] = {"Unk", "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	size_t i;
	datetime rawtime;
	//struct tm* timeinfo;

	// date/time check
	if_time_get(&rawtime);
	
	
	// Date: <SP> <dd> <SP> <mon> <SP> <yy> <SP> <hh> ":" <mm> ":" <ss> <SP> <zone> <CRLF>
	sprintf(header,"Date: %d %s %d %d:%d:%d%i\r\n",	rawtime.date,
																								month[rawtime.month],
																								rawtime.year,
																								rawtime.hour,
																								rawtime.minute,
																								rawtime.second,
																								(int32)rawtime.tz); 
	
	// From: <SP> <sender>  <SP> "<" <sender-email> ">" <CRLF>
	//if(!m_sMailFrom.size())
	//	throw ECSmtp(ECSmtp::UNDEF_MAIL_FROM);
	strcat(header,"From: ");
	strcat(header, from);
	strcat(header, "\r\n");

	// X-Mailer: <SP> <xmailer-app> <CRLF>
	//if(m_sXMailer.size())
	{
		strcat(header,"X-Mailer: ");
		strcat(header, "Kronmail v1.0");
		strcat(header, "\r\n");
	}

	// Reply-To: <SP> <reverse-path> <CRLF>
	strcat(header, "Reply-To: ");
	strcat(header, from);
	strcat(header, "\r\n");

	// X-Priority: <SP> <number> <CRLF>
	switch(ctx->priority)
	{
		case SMTP_XPRIORITY_HIGH:
			strcat(header,"X-Priority: 2 (High)\r\n");
			break;
		case SMTP_XPRIORITY_NORMAL:
			strcat(header,"X-Priority: 3 (Normal)\r\n");
			break;
		case SMTP_XPRIORITY_LOW:
			strcat(header,"X-Priority: 4 (Low)\r\n");
			break;
		default:
			strcat(header,"X-Priority: 3 (Normal)\r\n");
	}
	// To: <SP> <remote-user-mail> <CRLF>
	strcat(header,"To: ");
	strcat(header, to);
	strcat(header, "\r\n");


	// Subject: <SP> <subject-text> <CRLF>
	//if(!m_sSubject.size()) 
	//	strcat(header, "Subject:  ");
	//else
	strcat(header, "Subject: ");
	strcat(header, subject);
	strcat(header, "\r\n");
	
	// MIME-Version: <SP> 1.0 <CRLF>
	strcat(header,"MIME-Version: 1.0\r\n");
	//if(!Attachments.size())
	{ // no attachments
		strcat(header,"Content-type: text/plain; charset=US-ASCII\r\n");
		strcat(header,"Content-Transfer-Encoding: 7bit\r\n");
		strcat(header,"\r\n");
	}
}

////////////////////////////////////////////////////////////////////////////////
//        NAME: GetErrorText (friend function)
// DESCRIPTION: Returns the string for specified error code.
//   ARGUMENTS: CSmtpError ErrorId - error code
// USES GLOBAL: none
// MODIFIES GL: none 
//     RETURNS: error string
//      AUTHOR: Jakub Piwowarczyk
// AUTHOR/DATE: JP 2010-01-28
////////////////////////////////////////////////////////////////////////////////
uint8 * smtp_get_error_text(uint16 err)
{
	switch(err)
	{
		case SMTP_CSMTP_NO_ERROR:
			return "";
		case SMTP_WSA_STARTUP:
			return "Unable to initialise winsock2";
		case SMTP_WSA_VER:
			return "Wrong version of the winsock2";
		case SMTP_WSA_SEND:
			return "Function send() failed";
		case SMTP_WSA_RECV:
			return "Function recv() failed";
		case SMTP_WSA_CONNECT:
			return "Function connect failed";
		case SMTP_WSA_GETHOSTBY_NAME_ADDR:
			return "Unable to determine remote server";
		case SMTP_WSA_INVALID_SOCKET:
			return "Invalid winsock2 socket";
		case SMTP_WSA_HOSTNAME:
			return "Function hostname() failed";
		case SMTP_WSA_IOCTLSOCKET:
			return "Function ioctlsocket() failed";
		case SMTP_BAD_IPV4_ADDR:
			return "Improper IPv4 address";
		case SMTP_UNDEF_MSG_HEADER:
			return "Undefined message header";
		case SMTP_UNDEF_MAIL_FROM:
			return "Undefined mail sender";
		case SMTP_UNDEF_SUBJECT:
			return "Undefined message subject";
		case SMTP_UNDEF_RECIPIENTS:
			return "Undefined at least one reciepent";
		case SMTP_UNDEF_RECIPIENT_MAIL:
			return "Undefined recipent mail";
		case SMTP_UNDEF_LOGIN:
			return "Undefined user login";
		case SMTP_UNDEF_PASSWORD:
			return "Undefined user password";
		case SMTP_COMMAND_MAIL_FROM:
			return "Server returned error after sending MAIL FROM";
		case SMTP_COMMAND_EHLO:
			return "Server returned error after sending EHLO";
		case SMTP_COMMAND_AUTH_LOGIN:
			return "Server returned error after sending AUTH LOGIN";
		case SMTP_COMMAND_DATA:
			return "Server returned error after sending DATA";
		case SMTP_COMMAND_QUIT:
			return "Server returned error after sending QUIT";
		case SMTP_COMMAND_RCPT_TO:
			return "Server returned error after sending RCPT TO";
		case SMTP_MSG_BODY_ERROR:
			return "Error in message body";
		case SMTP_CONNECTION_CLOSED:
			return "Server has closed the connection";
		case SMTP_SERVER_NOT_READY:
			return "Server is not ready";
		case SMTP_SERVER_NOT_RESPONDING:
			return "Server not responding";
		case SMTP_FILE_NOT_EXIST:
			return "File not exist";
		case SMTP_MSG_TOO_BIG:
			return "Message is too big";
		case SMTP_BAD_LOGIN_PASS:
			return "Bad login or password";
		case SMTP_UNDEF_XYZ_RESPONSE:
			return "Undefined xyz SMTP response";
		case SMTP_LACK_OF_MEMORY:
			return "Lack of memory";
		case SMTP_TIME_ERROR:
			return "time() error";
		case SMTP_RECVBUF_IS_EMPTY:
			return "RecvBuf is empty";
		case SMTP_SENDBUF_IS_EMPTY:
			return "SendBuf is empty";
		case SMTP_OUT_OF_MSG_RANGE:
			return "Specified line number is out of message size";
		default:
			return "Undefined error id";
	}
}


