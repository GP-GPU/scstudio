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
 * $Id: pcap_load.cpp 438 2009-10-25 15:32:55Z gotthardp $
 */

// include pcap library
// under Windows WinPcap Developer's Pack http://www.winpcap.org/devel.htm
// under UN*X libpcap http://www.tcpdump.org
#include <pcap.h>

#include "pcap_load.h"

std::vector<MscPtr> Pcap::load_msc(const std::string &filename)
{
  std::vector<MscPtr> result;
  int res;

  pcap_t *fp;
  char errbuf[PCAP_ERRBUF_SIZE];
  // open the capture file
  if((fp = pcap_open_offline(filename.c_str(), errbuf)) == NULL)
  {
    print_report(RS_ERROR,
      stringize() << "Cannot open file '" << TOWSTRING(filename) << "'.");
    return result;
  }

  struct pcap_pkthdr *header;
  const u_char *pkt_data;
  // inspect all packets in the file
  while((res = pcap_next_ex(fp, &header, &pkt_data)) >= 0)
  {
    PacketDataUnit packet;
    packet.time_sent = packet.time_received =
      header->ts.tv_sec + header->ts.tv_usec / 1000000;
    packet.length = header->caplen;
    packet.data = pkt_data;

    inspect_ethernet(packet);
  }

  if(res == -1)
  {
    print_report(RS_ERROR,
      stringize() << "Error reading the packets: '" << pcap_geterr(fp));
  }

  pcap_close(fp);
  return result;
}

ImportFormatter::TransformationList Pcap::get_transformations(MscPtr msc) const
{
  ImportFormatter::TransformationList result;
  result.push_back(L"Beautify");
  return result;
}

// $Id: pcap_load.cpp 438 2009-10-25 15:32:55Z gotthardp $
