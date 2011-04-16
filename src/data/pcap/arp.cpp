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
 * $Id: arp.cpp 304 2009-09-10 18:35:08Z gotthardp $
 */

#include "pcap_load.h"

struct arp_header
{
  u_short ar_hrd; /* format of hardware address */
static const u_short ARPHRD_ETHER  = 1; /* ethernet hardware format */
static const u_short ARPHRD_FRELAY = 15; /* frame relay hardware format */
  u_short ar_pro; /* format of protocol address */
  u_char ar_hln; /* length of hardware address */
  u_char ar_pln; /* length of protocol address */
  u_short ar_op; /* one of: */
static const u_short ARPOP_REQUEST    = 1; /* request to resolve address */
static const u_short ARPOP_REPLY      = 2; /* response to previous request */
static const u_short ARPOP_REVREQUEST = 3; /* request protocol address given hardware */
static const u_short ARPOP_REVREPLY   = 4; /* response giving protocol address */
static const u_short ARPOP_INVREQUEST = 8; /* request to identify peer */
static const u_short ARPOP_INVREPLY   = 9; /* response identifying peer */
};

int Pcap::inspect_arp(const PacketDataUnit& packet)
{
  const arp_header *header = (const arp_header *)packet.data;

  return 0;
}

// $Id: arp.cpp 304 2009-09-10 18:35:08Z gotthardp $
