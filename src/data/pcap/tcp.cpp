/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: tcp.cpp 304 2009-09-10 18:35:08Z gotthardp $
 */

#include "pcap_load.h"

struct tcp_header
{
  u_short th_sport; /* source port */
  u_short th_dport; /* destination port */
  u_long th_seq; /* sequence number */
  u_long th_ack; /* acknowledgement number */
#ifdef HOST_IS_BIG_ENDIAN
  u_int th_off:4; /* data offset */
  u_int th_x2:4; /* (unused) */
#else
  u_int th_x2:4; /* (unused) */
  u_int th_off:4; /* data offset */
#endif
  u_char th_flags;
static const u_char TH_FIN  = 0x01; /* No more data from sender */
static const u_char TH_SYN  = 0x02; /* Synchronize sequence numbers */
static const u_char TH_RST  = 0x04; /* Reset the connection */
static const u_char TH_PUSH = 0x08; /* Push Function */
static const u_char TH_ACK  = 0x10; /* Acknowledgment field significant */
static const u_char TH_URG  = 0x20; /* Urgent Pointer field significant */
static const u_char TH_ECE  = 0x40; /* ECN capable during 3-way handshake */
static const u_char TH_CWR  = 0x80; /* Congestion Window Reduced */
  u_short th_win; /* window */
  u_short th_sum; /* checksum */
  u_short th_urp; /* urgent pointer */
};

int Pcap::inspect_tcp(const PacketDataUnit& packet)
{
  const tcp_header *header = (const tcp_header *)packet.data;

  return 0;
}

// $Id: tcp.cpp 304 2009-09-10 18:35:08Z gotthardp $
