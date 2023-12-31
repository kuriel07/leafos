/*****************************************************************************
 *  __________________    _________  _____            _____  .__         ._.
 *  \______   \______ \  /   _____/ /     \          /  _  \ |__| ____   | |
 *   |    |  _/|    |  \ \_____  \ /  \ /  \        /  /_\  \|  _/ __ \  | |
 *   |    |   \|    `   \/        /    Y    \      /    |    |  \  ___/   \|
 *   |______  /_______  /_______  \____|__  / /\   \____|__  |__|\___ |   __
 *          \/        \/        \/        \/  )/           \/        \/   \/
 *
 * This file is part of liBDSM. Copyright © 2014-2015 VideoLabs SAS
 *
 * Author: Julien 'Lta' BALLET <contact@lta.io>
 *
 * liBDSM is released under LGPLv2.1 (or later) and is also available
 * under a commercial license.
 *****************************************************************************
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <stdbool.h>

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <errno.h>

//#include "smb_defs.h"
//#include "bdsm_debug.h"
#include "../inc/netbios_defs.h"
#include "../inc/netbios_session.h"
#include "../inc/netbios_utils.h"

static int open_socket_and_connect(netbios_session *s)
{
    //if ((s->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    //    goto error;
    //if (connect(s->socket, (struct sockaddr *)&s->remote_addr, sizeof(s->remote_addr)) <0)
    //    goto error;
	uint8 inet_buffer[20];
	sprintf((char *)inet_buffer, "%d.%d.%d.%d", s->remote_addr.b_addr.b1,
										s->remote_addr.b_addr.b2, 
										s->remote_addr.b_addr.b3, 
										s->remote_addr.b_addr.b4);
	s->socket = if_net_tcp_open(s->netctx, inet_buffer, s->remote_port, NET_LISTEN);

    return DSM_SUCCESS;

error:
    BDSM_perror("netbios_session_new, open_socket: ");
    return DSM_ERROR_NETWORK;
}

static int        session_buffer_realloc(netbios_session *s, size_t new_size)
{
    void        *new_ptr;

    assert(s != NULL);

    /* BDSM_dbg("session_buffer_realloc: from %ld bytes to %ld bytes\n", */
    /*          s->packet_payload_size, new_size); */
	
    new_ptr  = os_realloc(s->packet, new_size);
    if (new_ptr != NULL)
    {
        s->packet_payload_size = new_size;
        s->packet = new_ptr;
        return 1;
    }
    os_free(s->packet);
    s->packet = NULL;
    return 0;
}

netbios_session *netbios_session_new(net_context_p ctx, size_t buf_size)
{
    netbios_session   *session;
    size_t            packet_size;

    session = (netbios_session *)os_alloc(sizeof(netbios_session));
    if (!session)
        return NULL;

	session->netctx = ctx;
    session->packet_payload_size = buf_size;
    packet_size = sizeof(netbios_session_packet) + session->packet_payload_size;
    session->packet = (netbios_session_packet *)os_alloc(packet_size);
    if (!session->packet) {
        os_free(session);
        return NULL;
    }
    session->socket = NULL;

    return session;
}

void netbios_session_destroy(netbios_session *s)
{
    if (!s)
        return;
    if (s->socket != NULL) {
        //closesocket(s->socket);
		if_net_tcp_close(s->socket);
	}

    os_free(s->packet);
    os_free(s);
}

int netbios_session_connect(uint32_t ip, netbios_session *s,
                            const char *name, int direct_tcp)
{
    ssize_t                   recv_size;
    char                      *encoded_name = NULL;
    uint16_t                  ports[2];
    unsigned int              nb_ports;
    bool                      opened = false;

    assert(s != NULL && s->packet != NULL);

    if (direct_tcp)
    {
        ports[0] = htons(NETBIOS_PORT_DIRECT);
        ports[1] = htons(NETBIOS_PORT_DIRECT_SECONDARY);
        nb_ports = 2;
    }
    else
    {
        ports[0] = htons(NETBIOS_PORT_SESSION);
        nb_ports = 1;
    }
    for (unsigned int i = 0; i < nb_ports && !opened; ++i)
    {
        //s->remote_addr.sin_port         = ports[i];
        //s->remote_addr.sin_family       = AF_INET;
        //s->remote_addr.sin_addr.s_addr  = ip;
		s->remote_addr.dw_addr = ip;
		s->remote_port = ports[i];
        opened = open_socket_and_connect(s) == DSM_SUCCESS;
    }
    if (!opened)
        goto error;

    if (!direct_tcp)
    {
        // Send the Session Request message
        netbios_session_packet_init(s);
        s->packet->opcode = NETBIOS_OP_SESSION_REQ;
        encoded_name = netbios_name_encode(name, 0, NETBIOS_FILESERVER);
        if (!netbios_session_packet_append(s, encoded_name, strlen(encoded_name) + 1))
            goto error;
        os_free(encoded_name);
        encoded_name = netbios_name_encode("LIBDSM", 0, NETBIOS_WORKSTATION);
        if (!netbios_session_packet_append(s, encoded_name, strlen(encoded_name) + 1))
            goto error;
        os_free(encoded_name);
        encoded_name = NULL;

        s->state = NETBIOS_SESSION_CONNECTING;
        if (!netbios_session_packet_send(s))
            goto error;

        // Now receiving the reply from the server.
        recv_size = netbios_session_packet_recv(s, NULL);
        if (recv_size < 0)
            goto error;

        // Reply was negative, we are not connected :(
        if (s->packet->opcode != NETBIOS_OP_SESSION_REQ_OK)
        {
            s->state = NETBIOS_SESSION_REFUSED;
            return 0;
        }
    }

    // Reply was OK or DirectTCP, a session has been established
    s->state = NETBIOS_SESSION_CONNECTED;
    return 1;

error:
    os_free(encoded_name);
    s->state = NETBIOS_SESSION_ERROR;
    return 0;
}

void              netbios_session_packet_init(netbios_session *s)
{
    assert(s != NULL);

    s->packet_cursor  = 0;
    s->packet->flags  = 0;
    s->packet->opcode = NETBIOS_OP_SESSION_MSG;
}

int               netbios_session_packet_append(netbios_session *s,
        const char *data, size_t size)
{
    char  *start;

    assert(s && s->packet);

    if (s->packet_payload_size - s->packet_cursor < size)
        if (!session_buffer_realloc(s, size + s->packet_cursor))
            return 0;

    start = ((char *)&s->packet->payload) + s->packet_cursor;
    memcpy(start, data, size);
    s->packet_cursor += size;

    return 1;
}

int               netbios_session_packet_send(netbios_session *s)
{
    ssize_t         to_send;
    ssize_t         sent;

    assert(s && s->packet && s->socket != NULL && s->state > 0);

    s->packet->length = htons(s->packet_cursor);
    to_send           = sizeof(netbios_session_packet) + s->packet_cursor;
    //sent              = send(s->socket, (void *)s->packet, to_send, 0);
	sent = if_net_tcp_send(s->socket, (void *)s->packet, to_send);

    if (sent != to_send)
    {
        BDSM_perror("netbios_session_packet_send: Unable to send (full?) packet");
        return 0;
    }

    return sent;
}

static ssize_t    netbios_session_get_next_packet(netbios_session *s)
{
    ssize_t         res;
    size_t          total, sofar;

    assert(s != NULL && s->packet != NULL && s->socket != NULL && s->state > 0);

    // Only get packet header and analyze it to get only needed number of bytes
    // needed for the packet. This will prevent losing a part of next packet
    total = sizeof(netbios_session_packet);
    sofar = 0;
    while (sofar < total)
    {
        //res = recv(s->socket, (uint8_t *)(s->packet) + sofar, total - sofar, 0);
		res = if_net_tcp_recv(s->socket, (uint8_t *)(s->packet) + sofar, total - sofar);
        if (res <= 0)
        {
            BDSM_perror("netbios_session_packet_recv: ");
            return -1;
        }
        sofar += res;
    }

    total  = ntohs(s->packet->length);
    total |= (s->packet->flags & 0x01) << 16;
    sofar  = 0;

    if (total + sizeof(netbios_session_packet) > s->packet_payload_size
     && !session_buffer_realloc(s, total + sizeof(netbios_session_packet)))
        return -1;

    //BDSM_dbg("Total = %ld, sofar = %ld\n", total, sofar);

    while (sofar < total)
    {
        //res = recv(s->socket, (uint8_t *)(s->packet) + sizeof(netbios_session_packet)
        //           + sofar, total - sofar, 0);
		res = if_net_tcp_recv(s->socket, (uint8_t *)(s->packet) + sizeof(netbios_session_packet)
                   + sofar, total - sofar);
        //BDSM_dbg("Total = %ld, sofar = %ld, res = %ld\n", total, sofar, res);

        if (res <= 0)
        {
            BDSM_perror("netbios_session_packet_recv: ");
            return -1;
        }
        sofar += res;
    }

    if (sofar > total)
    {
        BDSM_dbg("netbios_session_packet_recv: Packet size mismatch (%ld/%ld)\n",
                 sofar, total);
        return -1;
    }

    return sofar;
}

ssize_t           netbios_session_packet_recv(netbios_session *s, void **data)
{
    ssize_t         size;

    // ignore keepalive messages if needed
    do
    {
        size = netbios_session_get_next_packet(s);
    } while (size >= 0 && s->packet->opcode == NETBIOS_OP_SESSION_KEEPALIVE);

    if ((size >= 0) && (data != NULL))
        *data = (void *) s->packet->payload;

    return size;
}
