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
 * $Id: ethernet.cpp 304 2009-09-10 18:35:08Z gotthardp $
 */

#include "pcap_load.h"

// structure of a 10Mb/s Ethernet header
struct ethernet_header
{
  u_char ether_dhost[6];
  u_char ether_shost[6];
  u_short ether_type;
static const u_short ETHERTYPE_IP   = 0x0800; /* IP */ 
static const u_short ETHERTYPE_ARP  = 0x0806; /* ARP */ 
static const u_short ETHERTYPE_IPV6 = 0x86DD; /* IP version 6 */
};

int Pcap::inspect_ethernet(const PacketDataUnit& packet)
{
  const ethernet_header *header = (const ethernet_header *)packet.data;

  PacketDataUnit eth_data;
  eth_data.time_sent = packet.time_sent;
  eth_data.time_received = packet.time_received;
  eth_data.length = packet.length - sizeof(ethernet_header);
  eth_data.data = packet.data + sizeof(ethernet_header);

  switch(ntohs(header->ether_type))
  {
    case ethernet_header::ETHERTYPE_IP:
      inspect_ip(eth_data);
      break;

    case ethernet_header::ETHERTYPE_ARP:
      inspect_arp(eth_data);
      break;

    case ethernet_header::ETHERTYPE_IPV6:
      break;
  }

  return 0;
}

// $Id: ethernet.cpp 304 2009-09-10 18:35:08Z gotthardp $
