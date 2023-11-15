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
 *         Thomas Guillem <tguillem@videolabs.io>
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>

#include "../inc/netbios_ns.h"

//#include "bdsm_debug.h"
#include "../inc/netbios_defs.h"
#include "../inc/netbios_query.h"
#include "../inc/netbios_utils.h"
#include "../inc/netbios_queue.h"

#include "../../interfaces/inc/if_net.h"
#include "../../interfaces/inc/if_apis.h"

static int    ns_open_socket(netbios_ns *ns)
{
    int sock_opt;

    //if ((ns->socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    //    goto error;

    //sock_opt = 1;
    //if (setsockopt(ns->socket, SOL_SOCKET, SO_BROADCAST,
    //               (void *)&sock_opt, sizeof(sock_opt)) < 0)
    //    goto error;

    //sock_opt = 0;
    //if (setsockopt(ns->socket, IPPROTO_IP, IP_MULTICAST_LOOP,
    //               (void *)&sock_opt, sizeof(sock_opt)) < 0)
    //    goto error;

    //ns->addr.sin_family       = AF_INET;
    //ns->addr.sin_port         = htons(0);
    //ns->addr.sin_addr.s_addr  = INADDR_ANY;
    //if (bind(ns->socket, (struct sockaddr *)&ns->addr, sizeof(ns->addr)) < 0)
    //    goto error;
	net_conn_p conn = if_net_udp_open(ns->netctx, (uint8 *)"0.0.0.0", NETBIOS_PORT_NAME, NET_LISTEN);
	if(conn == NULL) return 0;
	ns->socket = conn;
    return 1;

error:
    BDSM_perror("netbios_ns_new, open_socket: ");
    return 0;
}



static int    ns_open_abort_pipe(netbios_ns *ns)
{
    //return pthread_mutex_init(&ns->abort_lock, NULL);
	return 0;
}

static void   ns_close_abort_pipe(netbios_ns *ns)
{
    //pthread_mutex_destroy(&ns->abort_lock);
}

static bool   netbios_ns_is_aborted(netbios_ns *ns)
{
    //pthread_mutex_lock(&ns->abort_lock);
    bool res = ns->aborted;
    //pthread_mutex_unlock(&ns->abort_lock);
    return res;
}

static void netbios_ns_abort(netbios_ns *ns)
{
    //pthread_mutex_lock(&ns->abort_lock);
    ns->aborted = true;
    //pthread_mutex_unlock(&ns->abort_lock);
}

static uint16_t query_type_nb = 0x2000;
static uint16_t query_type_nbstat = 0x2100;
static uint16_t query_class_in = 0x0100;
static uint16_t query_rname = 0x0CC0;
static uint16_t query_rd_len = 0x0600;
static uint16_t query_nb_flags = 0x0000;
static uint16_t query_name_flag = 0x0004;
static uint16_t query_server_flag = 0x0084;
uint8_c query_msbrowse_name[] = {0x01, 0x02, 0x5f, 0x5f, 0x4d, 0x53, 0x42, 0x52, 0x4f, 0x57, 0x53, 0x45, 0x5f, 0x5f, 0x02, 0x01};

static ssize_t netbios_ns_send_packet(netbios_ns* ns, netbios_query* q, uint32_t ip, uint16_t port)
{
    //struct sockaddr_in  addr;
	char inet_buffer[20];
	uint8 inet_addr[4];
	uint8 i;
    //addr.sin_addr.s_addr  = ip;
    //addr.sin_family       = AF_INET;
    //addr.sin_port         = htons(NETBIOS_PORT_NAME);
	ssize_t length =  sizeof(netbios_query_packet) + q->cursor;
	inet_addr[0] = (ip>>24) & 0xFF;
	inet_addr[1] = (ip>>16) & 0xFF;
	inet_addr[2] = (ip>>8) & 0xFF;
	inet_addr[3] = (uint8)(ip);
	sprintf(inet_buffer, "%d.%d.%d.%d", inet_addr[0], inet_addr[1], inet_addr[2], inet_addr[3] );
    //BDSM_dbg("Sending netbios packet to %s\n", inet_ntoa(addr.sin_addr));
	//if_net_udp_transmit(ns->netctx, inet_buffer, NETBIOS_PORT_NAME, (void *)q->packet,
    //              length, NULL);
	//ns->socket->remote_ip.dw_addr = ip;
	//ns->socket->remote_port = NETBIOS_PORT_NAME;
	//broadcast packet 3 times
	if(ip == 0xFFFFFFFF) {
	for(i=0;i<3;i++)
		if_net_udp_send(ns->socket, (void *)q->packet,
			length, inet_addr, port);
	} else{
		if_net_udp_send(ns->socket, (void *)q->packet,
			length, inet_addr, port);
	}
	return length;
}

static void netbios_ns_broadcast_packet(netbios_ns* ns, netbios_query* q)
{
    netbios_ns_send_packet(ns, q, INADDR_BROADCAST, NETBIOS_PORT_NAME);
}

static int netbios_ns_send_name_query(netbios_ns *ns,
                                      uint32_t ip,
                                      enum name_query_type type,
                                      const char *name,
                                      uint16_t query_flag)
{
    uint16_t            query_type;
    netbios_query       *q;

    assert(ns != NULL);

    switch (type)
    {
        case NAME_QUERY_TYPE_NB:
            query_type = query_type_nb;
            break;
        case NAME_QUERY_TYPE_NBSTAT:
            query_type = query_type_nbstat; // NBSTAT/IP;
            break;
        default:
            BDSM_dbg("netbios_ns_send_name_query: unknow name_query_type");
            return -1;
    }

    // Prepare packet
    q = netbios_query_new(34 + 4, 1, NETBIOS_OP_NAME_QUERY);
    if (query_flag)
        netbios_query_set_flag(q, query_flag, 1);

    // Append the queried name to the packet
    netbios_query_append(q, name, strlen(name) + 1);

    // Magic footer (i.e. Question type (Netbios) / class (IP)
    netbios_query_append(q, (const char *)&query_type, 2);
    netbios_query_append(q, (const char *)&query_class_in, 2);
    q->packet->queries = htons(1);

    // Increment transaction ID, not to reuse them
    q->packet->trn_id = htons(ns->last_trn_id + 1);

    if (ip != 0)
    {
        ssize_t sent = netbios_ns_send_packet(ns, q, ip, NETBIOS_PORT_NAME);
        if (sent < 0)
        {
            BDSM_perror("netbios_ns_send_name_query: ");
            netbios_query_destroy(q);
            return -1;
        }
        else
            BDSM_dbg("netbios_ns_send_name_query, name query sent for '*'.\n");
    }
    else
    {
        netbios_ns_broadcast_packet(ns, q);
    }

    netbios_query_destroy(q);

    ns->last_trn_id++; // Remember the last transaction id.
    return 0;
}

static int netbios_ns_send_reg_query(netbios_ns *ns,
                                      uint32_t ip,
                                      enum name_query_type type,
                                      const char *name,
                                      uint16_t query_flag)
{
    uint16_t            query_type;
    netbios_query       *q;
	uint32 ttl = 600;//seconds

    assert(ns != NULL);

    switch (type)
    {
        case NAME_QUERY_TYPE_NB:
            query_type = query_type_nb;
            break;
        case NAME_QUERY_TYPE_NBSTAT:
            query_type = query_type_nbstat; // NBSTAT/IP;
            break;
        default:
            BDSM_dbg("netbios_ns_send_name_query: unknow name_query_type");
            return -1;
    }

    // Prepare packet
    q = netbios_query_new(52 + 4, 1, 0);
    if (query_flag)
        netbios_query_set_flag(q, query_flag, 1);

    // Append the queried name to the packet
    netbios_query_append(q, name, strlen(name) + 1);

    // Magic footer (i.e. Question type (Netbios) / class (IP)
    netbios_query_append(q, (const char *)&query_type, 2);
    netbios_query_append(q, (const char *)&query_class_in, 2);
	
	//Resource Record
#if 1
    netbios_query_append(q, (const char *)&query_rname, 2);		//pointer to question name (2 bytes)
    netbios_query_append(q, (const char *)&query_type, 2);		//NB type (2 bytes)
    netbios_query_append(q, (const char *)&query_class_in, 2);	//IN class (2 bytes)
	ttl = htons(ttl);											
    netbios_query_append(q, (const char *)&ttl, 4);				//TTL 3200 seconds (4 bytes)
    netbios_query_append(q, (const char *)&query_rd_len, 2);	//resource data length size(nb_flags) + size(IPv4) - (2 bytes)
    netbios_query_append(q, (const char *)&query_nb_flags, 2);	//NB flags (B node)
	
	netbios_query_append(q, (const char *)ns->netctx->ipv4, 4);	//ip address of current NETBIOS registrant
#endif
    q->packet->queries = htons(1);
    q->packet->ar_count = htons(1);

    // Increment transaction ID, not to reuse them
    q->packet->trn_id = htons(ns->last_trn_id + 1);

    if (ip != 0)
    {
        ssize_t sent = netbios_ns_send_packet(ns, q, ip, NETBIOS_PORT_NAME);
        if (sent < 0)
        {
            BDSM_perror("netbios_ns_send_name_query: ");
            netbios_query_destroy(q);
            return -1;
        }
        else
            BDSM_dbg("netbios_ns_send_name_query, name query sent for '*'.\n");
    }
    else
    {
        netbios_ns_broadcast_packet(ns, q);
    }

    netbios_query_destroy(q);

    ns->last_trn_id++; // Remember the last transaction id.
    return 0;
}

static int netbios_ns_send_name_query_response(netbios_ns *ns,
                                      uint32_t ip,
									  uint16 port,
									  uint16 trn_id,
                                      enum name_query_type type,
                                      const char *name,		//name
                                      uint16_t query_flag)
{
    uint16_t            query_type;
    netbios_query       *q;
	uint16 rd_len = 154;
	uint8 num_of_names = 6;
	uint8 buf_name[16];
	uint32 ttl = 600;//seconds
	uint8 u8zero = 0;
	uint16 u16zero = 0;
	uint32 u32zero = 0;

    assert(ns != NULL);
	if(memcmp(ns->netctx->ipv4, (uint8 *)"\x0\x0\x0\x0", 4) == 0) {
		if_net_get_ipconfig(ns->netctx);
		if(memcmp(ns->netctx->ipv4, (uint8 *)"\x0\x0\x0\x0", 4) == 0) return -1;
	}

    switch (type)
    {
        case NAME_QUERY_TYPE_NB:
            query_type = query_type_nb;
            break;
        case NAME_QUERY_TYPE_NBSTAT:
            query_type = query_type_nbstat; // NBSTAT/IP;
            break;
        default:
            BDSM_dbg("netbios_ns_send_name_query: unknow name_query_type");
            return -1;
    }

    // Prepare packet
    q = netbios_query_new(250 + 4, 1, 0);
    if (query_flag)
        netbios_query_set_flag(q, query_flag, 1);
    // Append the queried name to the packet
    netbios_query_append(q, name, strlen(name) + 1);

    // Magic footer (i.e. Question type (Netbios) / class (IP)
    netbios_query_append(q, (const char *)&query_type, 2);
    netbios_query_append(q, (const char *)&query_class_in, 2);
	
	//Resource Record
#if 1
    ttl = end_swap32(ttl);											
    netbios_query_append(q, (const char *)&ttl, 4);				//TTL 3200 seconds (4 bytes)
	rd_len = htons(rd_len);
    netbios_query_append(q, (const char *)&rd_len, 2);	//resource data length size(nb_flags) + size(IPv4) - (2 bytes)
    netbios_query_append(q, (const char *)&num_of_names, 1);	//number of names
	//server service
	memset(buf_name, 0x20, sizeof(buf_name));		//server flag	
	memcpy(buf_name, ns->name, strlen(ns->name));
	buf_name[15] = 0x20;
	netbios_query_append(q, (const char *)buf_name, 0x10);	//device name
    netbios_query_append(q, (const char *)&query_name_flag, 2);				//name flag
	//workstation
	memset(buf_name, 0x20, sizeof(buf_name));		//workstation flag
	memcpy(buf_name, ns->name, strlen(ns->name));
	buf_name[15] = 0;
	netbios_query_append(q, (const char *)buf_name, 0x10);	//device name
    netbios_query_append(q, (const char *)&query_name_flag, 2);				//name flag
	//workstation/redirector
	memset(buf_name, 0x20, sizeof(buf_name));		//workstation flag
	memcpy(buf_name, "ORBLEAF", strlen("ORBLEAF"));
	buf_name[15] = 0;
	netbios_query_append(q, (const char *)buf_name, 0x10);	//device name
    netbios_query_append(q, (const char *)&query_server_flag, 2);				//name flag
	//browser election service
	memset(buf_name, 0x20, sizeof(buf_name));		//browser election flag
	memcpy(buf_name, "ORBLEAF", strlen("ORBLEAF"));
	buf_name[15] = 0x1E;
	netbios_query_append(q, (const char *)buf_name, 0x10);	//device name
    netbios_query_append(q, (const char *)&query_server_flag, 2);				//name flag
	//local master browser
	memset(buf_name, 0x20, sizeof(buf_name));		//local master flag
	memcpy(buf_name, "ORBLEAF", strlen("ORBLEAF"));
	buf_name[15] = 0x1D;
	netbios_query_append(q, (const char *)buf_name, 0x10);	//device name
    netbios_query_append(q, (const char *)&query_name_flag, 2);				//name flag
	
	//MSBROWSE
	memcpy(buf_name, query_msbrowse_name, sizeof(query_msbrowse_name));
	netbios_query_append(q, (const char *)buf_name, 0x10);	//device name
    netbios_query_append(q, (const char *)&query_server_flag, 2);				//name flag
	
    netbios_query_append(q, (const char *)ns->netctx->mac, 6);				//name flag
    netbios_query_append(q, (const char *)&u8zero, 1);				//jumper
    netbios_query_append(q, (const char *)&u8zero, 1);				//test flag
    netbios_query_append(q, (const char *)&u16zero, 2);				//version number
    netbios_query_append(q, (const char *)&u16zero, 2);				//statistic
    netbios_query_append(q, (const char *)&u16zero, 2);				//crcs
    netbios_query_append(q, (const char *)&u16zero, 2);				//alignment error
    netbios_query_append(q, (const char *)&u16zero, 2);				//collissions
    netbios_query_append(q, (const char *)&u16zero, 2);				//send aborts
    netbios_query_append(q, (const char *)&u32zero, 4);				//good sends
    netbios_query_append(q, (const char *)&u32zero, 4);				//good receive
    netbios_query_append(q, (const char *)&u16zero, 2);				//retransmit
    netbios_query_append(q, (const char *)&u16zero, 2);				//no resource condition
    netbios_query_append(q, (const char *)&u16zero, 2);				//command block
    netbios_query_append(q, (const char *)&u16zero, 2);				//pending session
    netbios_query_append(q, (const char *)&u16zero, 2);				//max session packet
    netbios_query_append(q, (const char *)&u16zero, 2);				//data packet size
    netbios_query_append(q, (const char *)"\x0\x0\x0\x0\x0\x0", 6);				//padding bytes
	
	//netbios_query_append(q, (const char *)ns->netctx->ipv4, 4);	//ip address of current NETBIOS registrant
#endif
    //q->packet->queries = htons(1);
    //q->packet->ar_count = htons(1);
	q->packet->answers = htons(1);
	//q->packet->trn_id = htons(ntohs(trn_id) + 1);
	q->packet->trn_id = trn_id;

    // Increment transaction ID, not to reuse them
    //q->packet->trn_id = htons(ns->last_trn_id + 1);

    if (ip != 0)
    {
        ssize_t sent = netbios_ns_send_packet(ns, q, ip, port);
        if (sent < 0)
        {
            BDSM_perror("netbios_ns_send_name_query: ");
            netbios_query_destroy(q);
            return -1;
        }
        else
            BDSM_dbg("netbios_ns_send_name_query, name query sent for '*'.\n");
    }
    else
    {
        netbios_ns_broadcast_packet(ns, q);
    }

    netbios_query_destroy(q);

    ns->last_trn_id++; // Remember the last transaction id.
    return 0;
}

static int netbios_ns_handle_query(netbios_ns *ns, size_t size,
                                   bool check_trn_id, uint32_t recv_ip, uint16 recv_port,
                                   netbios_ns_name_query *out_name_query)
{
    netbios_query_packet *q;
    uint8_t name_size;
    uint16_t *p_type, type;
    uint16_t *p_data_length, data_length;
    char     *p_data;

    // check for packet size
    if (size < sizeof(netbios_query_packet))
    {
        BDSM_dbg("netbios_ns_handle_query, invalid size !\n");
        return -1;
    }

    q = (netbios_query_packet *)ns->buffer;
    if (check_trn_id)
    {
        // check if trn_id corresponds
        if (ntohs(q->trn_id) != ns->last_trn_id) {
            BDSM_dbg("netbios_ns_handle_query, invalid trn_id: %d vs %d\n",
                     ntohs(q->trn_id), ns->last_trn_id);
            return -1;
        }
    }

    if (!out_name_query)
        return 0;

    // get Name size, should be 0x20
    if (size < sizeof(netbios_query_packet) + 1)
        return -1;
    name_size = q->payload[0];
    if (name_size != 0x20)
        return -1;

	//check for name query request here, reponse with name query response (NBSTAT)
	if((ntohs(q->flags) & 0xF800) == 0x0000) {		//name query request detection
		//generate name query response for this machine here
		netbios_ns_send_name_query_response(ns, recv_ip, recv_port, q->trn_id, NAME_QUERY_TYPE_NBSTAT, q->payload, 
			NETBIOS_FLAG_QUERY | NETBIOS_FLAG_AUTHOR);
		return 0;
	}
    // get type and data_length
    if (size < sizeof(netbios_query_packet) + name_size + 11) {
		return -1;
	}
	
	
    p_type = (uint16_t *) (q->payload + name_size + 2);
    type = *p_type;
    p_data_length = (uint16_t *) (q->payload + name_size + 10);
    data_length = ntohs(*p_data_length);

    if (size < sizeof(netbios_query_packet) + name_size + 12 + data_length)
        return -1;
    p_data = q->payload + name_size + 12;
	
    if (type == query_type_nb) {
        out_name_query->type = NAME_QUERY_TYPE_NB;
        out_name_query->u.nb.ip = recv_ip;
		out_name_query->result = ntohs(q->flags) & 0x0F;
		out_name_query->opcode = (ntohs(q->flags) >> 11) & 0x0F;
    } else if (type == query_type_nbstat) {
        uint8_t name_count;
        const char *names = NULL;
        const char *group = NULL, *name = NULL;;

        // get the number of names
        if (data_length < 1)
            return -1;
        name_count = *(p_data);
        names = p_data + 1;

        if (data_length < name_count * 18)
            return -1;

        // first search for a group in the name list
        for (uint8_t name_idx = 0; name_idx < name_count; name_idx++)
        {
            const char *current_name = names + name_idx * 18;
            uint16_t current_flags = (current_name[16] << 8) | current_name[17];
            if (current_flags & NETBIOS_NAME_FLAG_GROUP) {
                group = current_name;
                break;
            }
        }
        // then search for file servers
        for (uint8_t name_idx = 0; name_idx < name_count; name_idx++)
        {
            const char *current_name = names + name_idx * 18;
            char current_type = current_name[15];
            uint16_t current_flags = (current_name[16] << 8) | current_name[17];

            if (current_flags & NETBIOS_NAME_FLAG_GROUP)
                continue;
            if (current_type == NETBIOS_WORKSTATION)
            {
                name = current_name;
                BDSM_dbg("netbios_ns_handle_query, Found name: '%.*s' in group: '%.*s'\n",
                         NETBIOS_NAME_LENGTH, name, NETBIOS_NAME_LENGTH, group);
                break;
            }
        }

        if (name)
        {
            out_name_query->type = NAME_QUERY_TYPE_NBSTAT;
			out_name_query->result = ntohs(q->flags) & 0x0F;
			out_name_query->opcode = (ntohs(q->flags) >> 11) & 0x0F;
            out_name_query->u.nbstat.name = name;
            out_name_query->u.nbstat.group = group;
            out_name_query->u.nbstat.type = NETBIOS_FILESERVER;
        }
    }

    return 0;
}

static ssize_t netbios_ns_recv(netbios_ns *ns,
                               struct timeval *timeout,
                               net_addr *out_addr,
                               bool check_trn_id,
                               uint32_t wait_ip,
                               netbios_ns_name_query *out_name_query)
{
    void * sock;
	ssize_t size;

    assert(ns != NULL);

    sock = ns->socket;
#ifdef HAVE_PIPE
    int abort_fd =  ns->abort_pipe[0];
#else
    int abort_fd = -1;
#endif

    if (out_name_query)
        out_name_query->type = NAME_QUERY_TYPE_INVALID;
	
    while (true) {
		if_net_set_timeout(ns->socket, ((timeout->tv_sec * 1000) + timeout->tv_usec));
		
		//fd_set read_fds, error_fds;
		//int res, nfds;

		//FD_ZERO(&read_fds);
		//FD_ZERO(&error_fds);
		//FD_SET(sock, &read_fds);
#ifdef HAVE_PIPE
		//FD_SET(abort_fd, &read_fds);
#endif
		//FD_SET(sock, &error_fds);
		//nfds = (sock > abort_fd ? sock : abort_fd) + 1;

		//res = select(nfds, &read_fds, 0, &error_fds, timeout);

		//if (res < 0)
		//	goto error;
		//if (FD_ISSET(sock, &error_fds))
		//	goto error;
#ifdef HAVE_PIPE
		if (FD_ISSET(abort_fd, &read_fds))
			return -1;
#else
		if (netbios_ns_is_aborted(ns))
			return -1;
#endif
		else {
			//struct sockaddr_in addr;
			//socklen_t addr_len = sizeof(struct sockaddr_in);
			size = if_net_udp_recv(sock, ns->buffer, RECV_BUFFER_SIZE);
			//size = recvfrom(sock, ns->buffer, RECV_BUFFER_SIZE, 0,
			//			   (struct sockaddr *)&addr, &addr_len);
			if (size == 0)
				return 0;		//timeout
			if (wait_ip != 0)
			{
				// wait for a reply from a specific ip
				
				if (wait_ip != ns->socket->remote_ip.dw_addr)
				{
				//	BDSM_dbg("netbios_ns_recv, invalid ip");
					continue;
				}
			}

			if (netbios_ns_handle_query(ns, (size_t)size, check_trn_id,
										ns->socket->remote_ip.dw_addr, 
										ns->socket->remote_port,
										out_name_query) == -1)
			{
				BDSM_dbg("netbios_ns_recv, invalid query\n");
				continue;
			}

			//if (out_addr)
			//	*out_addr = addr;
			if(out_addr != NULL)
				memcpy(out_addr, &ns->socket->remote_ip, sizeof(net_addr));
			return size;
		}
		//else
		//	return 0;
	}
error:
    BDSM_perror("netbios_ns_recv: ");
    return -1;
}

static void netbios_ns_copy_name(char *dest, const char *src)
{
    memcpy(dest, src, NETBIOS_NAME_LENGTH);
    dest[NETBIOS_NAME_LENGTH] = 0;

    for (int i = 1; i < NETBIOS_NAME_LENGTH; i++ )
      if (dest[NETBIOS_NAME_LENGTH - i] == ' ')
        dest[NETBIOS_NAME_LENGTH - i] = 0;
      else
        break;
}

static void netbios_ns_entry_set_name(netbios_ns_entry *entry,
                                      const char *name, const char *group,
                                      char type)
{
    if (name != NULL)
        netbios_ns_copy_name(entry->name, name);
    if (group != NULL)
        netbios_ns_copy_name(entry->group, group);

    entry->type = type;
    entry->flag |= NS_ENTRY_FLAG_VALID_NAME;
}

static netbios_ns_entry *netbios_ns_entry_add(netbios_ns *ns, uint32_t ip)
{
    netbios_ns_entry  *entry;

    entry = os_alloc(sizeof(netbios_ns_entry));
    if (!entry)
        return NULL;

    entry->address.dw_addr = ip;
    entry->flag |= NS_ENTRY_FLAG_VALID_IP;

    TAILQ_INSERT_HEAD(&ns->entry_queue, entry, next);

    return entry;
}

// Find an entry in the list. Search by name if name is not NULL,
// or by ip otherwise
netbios_ns_entry *netbios_ns_entry_find(netbios_ns *ns, const char *by_name,
                                               uint32_t ip)
{
    netbios_ns_entry  *iter;

    assert(ns != NULL);

    TAILQ_FOREACH(iter, &ns->entry_queue, next)
    {
        if (by_name != NULL)
        {
            if (iter->flag & NS_ENTRY_FLAG_VALID_NAME
                && !strncmp(by_name, iter->name, NETBIOS_NAME_LENGTH))
                return iter;
        }
        else if (iter->flag & NS_ENTRY_FLAG_VALID_IP
                 && iter->address.dw_addr == ip)
            return iter;
    }

    return NULL;
}

void netbios_ns_entry_clear(netbios_ns *ns)
{
    netbios_ns_entry  *entry, *entry_next;

    assert(ns != NULL);

    for (entry = TAILQ_FIRST(&ns->entry_queue);
         entry != NULL; entry = entry_next)
    {
        entry_next = TAILQ_NEXT(entry, next);
        TAILQ_REMOVE(&ns->entry_queue, entry, next);
        os_free(entry);
    }
}

netbios_ns  *netbios_ns_new(net_context_p ctx)
{
    netbios_ns  *ns;

    ns = os_alloc(sizeof(netbios_ns));
    if (!ns)
        return NULL;

#ifdef HAVE_PIPE
    // Don't initialize this in ns_open_abort_pipe, as it would lead to
    // fd 0 to be closed (twice) in case of ns_open_socket error
    ns->abort_pipe[0] = ns->abort_pipe[1] = -1;
#endif
	ns->netctx = ctx;
    if (!ns_open_socket(ns) || ns_open_abort_pipe(ns) == -1)
    {
        netbios_ns_destroy(ns);
        return NULL;
    }

    TAILQ_INIT(&ns->entry_queue);
    ns->last_trn_id   = rand();

    return ns;
}

void          netbios_ns_destroy(netbios_ns *ns)
{
    if (!ns)
        return;

    netbios_ns_entry_clear(ns);

    if (ns->socket != NULL) {
        //closesocket(ns->socket);
		if_net_udp_close(ns->socket);
	}

    ns_close_abort_pipe(ns);

    os_free(ns);
}

int      netbios_ns_resolve(netbios_ns *ns, const char *name, char type, uint32_t *addr)
{
    netbios_ns_entry    *cached;
    struct timeval      timeout;
    char                *encoded_name;
    ssize_t             recv;
    netbios_ns_name_query name_query;

    assert(ns != NULL && !ns->discover_started);

    if ((cached = netbios_ns_entry_find(ns, name, 0)) != NULL)
    {
        *addr = cached->address.dw_addr;
        return 0;
    }

    if ((encoded_name = netbios_name_encode(name, 0, type)) == NULL)
        return -1;

    if (netbios_ns_send_name_query(ns, 0, NAME_QUERY_TYPE_NB, encoded_name,
                                   NETBIOS_FLAG_RECURSIVE |
                                   NETBIOS_FLAG_BROADCAST) == -1)
    {
        os_free(encoded_name);
        return -1;

    }
    os_free(encoded_name);

    // Now wait for a reply and pray
    timeout.tv_sec = 2;
    timeout.tv_usec = 420;
	//if_net_set_timeout(ns->socket, 2420);
    recv = netbios_ns_recv(ns, &timeout, NULL, true, 0, &name_query);

    if (recv < 0)
        BDSM_perror("netbios_ns_resolve:");
    else
    {
        if (name_query.type == NAME_QUERY_TYPE_NB)
        {
            *addr = name_query.u.nb.ip;
            BDSM_dbg("netbios_ns_resolve, received a reply for '%s', ip: 0x%X!\n", name, *addr);
            return 0;
        } else
            BDSM_dbg("netbios_ns_resolve, wrong query type received\n");
    }

    return -1;
}

// Perform inverse name resolution. Grap an IP and return the first <20> field
// returned by the host
static netbios_ns_entry *netbios_ns_inverse_internal(netbios_ns *ns, uint32_t ip)
{
    netbios_ns_entry  *cached;
    struct timeval      timeout;
    ssize_t             recv;
    netbios_ns_name_query name_query;
    netbios_ns_entry *entry;

    if ((cached = netbios_ns_entry_find(ns, NULL, ip)) != NULL)
        return cached;

    if (netbios_ns_send_name_query(ns, ip, NAME_QUERY_TYPE_NBSTAT,
                                   name_query_broadcast, 0) == -1)
        goto error;

    // Now wait for a reply and pray
    timeout.tv_sec = 1;
    timeout.tv_usec = 500;
    recv = netbios_ns_recv(ns, &timeout, NULL, true, ip, &name_query);

    if (recv <= 0)
        goto error;

    if (name_query.type != NAME_QUERY_TYPE_NBSTAT)
    {
        BDSM_dbg("netbios_ns_inverse, wrong query type received\n");
        goto error;
    }
    //else
        //BDSM_dbg("netbios_ns_inverse, received a reply for '%s' !\n",
        //         inet_ntoa(*(struct in_addr *)&ip));

    entry = netbios_ns_entry_add(ns, ip);
    if (entry) {
        netbios_ns_entry_set_name(entry, name_query.u.nbstat.name,
                                  name_query.u.nbstat.group,
                                  name_query.u.nbstat.type);
		
		ns->discover_callbacks.pf_on_entry_added(
				ns->discover_callbacks.p_opaque, entry);
	}
    return entry;
error:
    BDSM_perror("netbios_ns_inverse: ");
    return NULL;
}

const char *netbios_ns_inverse(netbios_ns *ns, uint32_t ip)
{
    assert(ns != NULL && ip != 0 && !ns->discover_started);
    netbios_ns_entry *entry = netbios_ns_inverse_internal(ns, ip);
    return entry ? entry->name : NULL;
}

const char *netbios_ns_entry_name(netbios_ns_entry *entry)
{
    return entry ? entry->name : NULL;
}

const char *netbios_ns_entry_group(netbios_ns_entry *entry)
{
    return entry ? entry->group : NULL;
}

uint32_t netbios_ns_entry_ip(netbios_ns_entry *entry)
{
    return entry ? entry->address.dw_addr : 0;
}

char netbios_ns_entry_type(netbios_ns_entry *entry)
{
    return entry ? entry->type : -1;
}


int netbios_ns_register(netbios_ns *ns, uint32_t ip)
{
	int ret = -1;
    netbios_ns_entry    *cached;
    struct timeval      timeout;
    char                *encoded_name;
    ssize_t             recv;
    netbios_ns_name_query reg_query;
	uint16 query_op = NETBIOS_OP_REG_QUERY;

    assert(ns != NULL && !ns->discover_started);
	restart_registration:
	if(memcmp(ns->netctx->ipv4, (uint8 *)"\x0\x0\x0\x0", 4) == 0) {
		if_net_get_ipconfig(ns->netctx);
		if(memcmp(ns->netctx->ipv4, (uint8 *)"\x0\x0\x0\x0", 4) == 0) return -1;
	}
    if ((encoded_name = netbios_name_encode(ns->name, 0, 0x00)) == NULL)
        return -1;

    if (netbios_ns_send_reg_query(ns, ip, NAME_QUERY_TYPE_NB, encoded_name,
								   query_op |
                                   NETBIOS_FLAG_RECURSIVE |
                                   NETBIOS_FLAG_BROADCAST) == -1)
    {
        os_free(encoded_name);
        return -1;

    }
    os_free(encoded_name);

    // Now wait for a reply and pray
    timeout.tv_sec = 5;
    timeout.tv_usec = 420;
	//if_net_set_timeout(ns->socket, 2420);
    recv = netbios_ns_recv(ns, &timeout, NULL, true, 0, &reg_query);

    if (recv <= 0)
        BDSM_perror("netbios_ns_resolve:");
    else
    {
		
        if (reg_query.type == NAME_QUERY_TYPE_NB)
        {
			switch(reg_query.result) {
				case 0:		//positive response
					if(query_op == NETBIOS_OP_REL_QUERY) {		//check if op is release
						query_op = NETBIOS_OP_REG_QUERY;		//re-register netbios name
						goto restart_registration;
					}
					ret = 0;
					break;
				case 0x01:		//format error
				case 0x02:		//server error
				case 0x04:		//unsupported request error
				case 0x05:		//RFS error
					break;
				case 0x06:		//already owned by another node
				case 0x07:		//name in conflict
					if(query_op == NETBIOS_OP_REG_QUERY) {
						query_op = NETBIOS_OP_REL_QUERY;		//release demand
						goto restart_registration;
					}
					break;
					
			}
        }
    }
    return ret;
}

static void *netbios_ns_discover_thread()
{
    netbios_ns *ns = (netbios_ns *) os_get_context();
	time_t now;
	struct timeval      timeout;
    netbios_ns_name_query name_query;
	net_addr  	recv_addr;
	int                 res;
	uint32_t ip;
	char * name;
    //char                *encoded_name;
	//register this machine first
	//if(strlen(ns->name) != 0) {
		//netbios_ns_register(ns);
		//if ((encoded_name = netbios_name_encode(ns->name, 0, 0x00)) == NULL)
		//	encoded_name = name_query_broadcast;
	//}
    while (true)
    {
        const int remove_timeout = 600;		//every 10 minutes
        netbios_ns_entry  *entry, *entry_next;

        if (netbios_ns_is_aborted(ns))
            return NULL;

		//if(strlen(ns->name) != 0) {
		//	netbios_ns_register(ns);
		//}
        // check if cached entries timeout, the timeout value is 5 times the
        // broadcast timeout.
        now = (if_sys_tick() / 1000);		//value in second
        for (entry = TAILQ_FIRST(&ns->entry_queue);
             entry != NULL; entry = entry_next)
        {
            entry_next = TAILQ_NEXT(entry, next);
            if (now - entry->last_time_seen > (now + remove_timeout))
            {
                if (entry->flag & NS_ENTRY_FLAG_VALID_NAME)
                {
                    BDSM_dbg("Discover: on_entry_removed: %s\n", entry->name);
                    ns->discover_callbacks.pf_on_entry_removed(
                            ns->discover_callbacks.p_opaque, entry);
                }
                TAILQ_REMOVE(&ns->entry_queue, entry, next);
                os_free(entry);
            }
        }

        // send broadbast NETBIOS_FLAG_RECURSIVE | NETBIOS_FLAG_BROADCAST
        if (netbios_ns_send_name_query(ns, 0, NAME_QUERY_TYPE_NB,
                                       name_query_broadcast, 0) == -1)
            return NULL;
		
        while (true)
        {

            timeout.tv_sec = ns->discover_broadcast_timeout;
			//timeout.tv_sec = 5;
            timeout.tv_usec = 0;

            // receive NB or NBSTAT answers
            res = netbios_ns_recv(ns, ns->discover_broadcast_timeout == 0 ?
                                  NULL : &timeout,
                                  &recv_addr,
                                  false, 0, &name_query);
            // error or abort
            if (res == -1)
                return NULL;

            // timeout reached, broadcast again
            if (res == 0)
                break;

            now = (if_sys_tick() / 1000);

            if (name_query.type == NAME_QUERY_TYPE_NB)
            {
				switch(name_query.opcode) {
					case 0:			//type query
						ip = name_query.u.nb.ip;
						entry = netbios_ns_entry_find(ns, NULL, ip);
						//entry = netbios_ns_inverse_internal(ns, ip);
						if (!entry)
						{
							//
							entry = netbios_ns_entry_add(ns, ip);
							if (!entry)
								return NULL;
						}
						entry->last_time_seen = now;

						// if entry is already valid, don't send NBSTAT query
						if (entry->flag & NS_ENTRY_FLAG_VALID_NAME)
							continue;

						// send NBSTAT query
						if (netbios_ns_send_name_query(ns, ip, NAME_QUERY_TYPE_NBSTAT,
													   name_query_broadcast, 0) == -1)
							return NULL;
						break;
					case 0x08:		//refresh mechanism, try re-register the device
						//if(strlen(ns->name) != 0) {
						//	netbios_ns_register(ns);
						//}
						break;
				}

            }
            else if (name_query.type == NAME_QUERY_TYPE_NBSTAT)
            {
                bool send_callback;

                entry = netbios_ns_entry_find(ns, NULL,
                                              recv_addr.dw_addr);

                // don't ignore NBSTAT answers that didn't answered to NB query first.
                if (!entry) {
					entry = netbios_ns_entry_add(ns, recv_addr.dw_addr);
					if(entry == NULL) break;
				}
                //continue;

                entry->last_time_seen = now;

                send_callback = !(entry->flag & NS_ENTRY_FLAG_VALID_NAME);

                netbios_ns_entry_set_name(entry, name_query.u.nbstat.name,
                                          name_query.u.nbstat.group,
                                          name_query.u.nbstat.type);
                if (send_callback)
                    ns->discover_callbacks.pf_on_entry_added(
                            ns->discover_callbacks.p_opaque, entry);
            }
        }
        if (ns->discover_broadcast_timeout == 0)
            break;
    }
    return NULL;
}

int netbios_ns_discover_start(netbios_ns *ns,
                              unsigned int broadcast_timeout,
                              netbios_ns_discover_callbacks *callbacks)
{
    if (ns->discover_started || !callbacks)
        return -1;

    ns->discover_callbacks = *callbacks;
    ns->discover_broadcast_timeout = broadcast_timeout;
    //if (pthread_create(&ns->discover_thread, NULL,
    //                   netbios_ns_discover_thread, ns) != 0)
    //    return -1;
	//ns->dis
	ns->discover_thread = os_create_task(ns, netbios_ns_discover_thread, "nbios", 137, 4096);
	if(ns->discover_thread == NULL) 
		return -1;
    ns->discover_started = true;

    return 0;
}

int netbios_ns_discover_stop(netbios_ns *ns)
{
    if (ns->discover_started)
    {
        netbios_ns_abort(ns);
        //pthread_join(ns->discover_thread, NULL);
		os_kill_task(ns->discover_thread);
        ns->discover_started = false;

        return 0;
    }
    else
        return -1;
}
