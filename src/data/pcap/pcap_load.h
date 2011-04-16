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
 * $Id: pcap_load.h 438 2009-10-25 15:32:55Z gotthardp $
 */

#ifndef _SCPCAP_PCAP_IMPORT_H_
#define _SCPCAP_PCAP_IMPORT_H_

#ifdef WIN32
#include <windows.h>
#define uint16_t UINT16
#define uint32_t UINT32
#endif

#include "data/formatter.h"
#include "data/pcap/export.h"

class SCPCAP_EXPORT Pcap : public Formatter, public ImportFormatter
{
public:
  //! file extension used to distinguish this format
  // note: DLL in Windows cannot return pointers to static data
  virtual std::string get_extension() const
  { return "pcap"; }
  //! human readable description of this format
  virtual std::string get_description() const
  { return "Wireshark (libpcap) packet trace"; }

  //! import MSC document
  virtual std::vector<MscPtr> load_msc(const std::string &filename);
  //! Returns a list of transformation for this format.
  virtual TransformationList get_transformations(MscPtr msc) const;

protected:
  struct PacketDataUnit
  {
    double time_sent;
    double time_received;
    size_t length;
    const u_char* data;
  };

  int inspect_ethernet(const PacketDataUnit& packet);
  int inspect_arp(const PacketDataUnit& packet);
  int inspect_ip(const PacketDataUnit& packet);
  int inspect_icmp(const PacketDataUnit& packet);
  int inspect_tcp(const PacketDataUnit& packet);
  int inspect_udp(const PacketDataUnit& packet);
};

#endif /* _SCPCAP_PCAP_IMPORT_H_ */

// $Id: pcap_load.h 438 2009-10-25 15:32:55Z gotthardp $
