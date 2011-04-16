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
 * $Id: udp.cpp 304 2009-09-10 18:35:08Z gotthardp $
 */

#include "pcap_load.h"

struct udp_header
{
  u_short uh_sport; /* source port */
  u_short uh_dport; /* destination port */
  u_short uh_ulen; /* udp length */
  u_short uh_sum; /* udp checksum */
};

int Pcap::inspect_udp(const PacketDataUnit& packet)
{
  const udp_header *header = (const udp_header *)packet.data;

  return 0;
}

// $Id: udp.cpp 304 2009-09-10 18:35:08Z gotthardp $
