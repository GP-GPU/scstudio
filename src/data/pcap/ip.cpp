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
 * $Id: ip.cpp 304 2009-09-10 18:35:08Z gotthardp $
 */

#include "pcap_load.h"

struct ip_header
{
#ifdef HOST_IS_BIG_ENDIAN
  u_int ip_v:4;  /* version */
  u_int ip_hl:4; /* header length */
#else
  u_int ip_hl:4; /* header length */
  u_int ip_v:4;  /* version */
#endif
  u_char ip_tos; /* type of service */
  u_short ip_len; /* total length */
  u_short ip_id; /* identification */
  u_short ip_off; /* fragment offset field */
static const u_short IP_RF = 0x8000; /* reserved fragment flag */
static const u_short IP_DF = 0x4000; /* dont fragment flag */
static const u_short IP_MF = 0x2000; /* more fragments flag */
static const u_short IP_OFFMASK = 0x1fff; /* mask for fragmenting bits */
  u_char ip_ttl; /* time to live */
  u_char ip_p; /* protocol (RFC 1700) */
static const u_char PROTO_IP         = 0; /* dummy for IP */
static const u_char PROTO_ICMP       = 1; /* control message protocol */
static const u_char PROTO_IGMP       = 2; /* group mgmt protocol */
static const u_char PROTO_GGP        = 3; /* gateway^2 (deprecated) */
static const u_char PROTO_IPIP       = 4; /* IP encapsulation in IP */
static const u_char PROTO_TCP        = 6; /* tcp */
static const u_char PROTO_ST         = 7; /* Stream protocol II */
static const u_char PROTO_EGP        = 8; /* exterior gateway protocol */
static const u_char PROTO_PIGP       = 9; /* private interior gateway */
static const u_char PROTO_RCCMON    = 10; /* BBN RCC Monitoring */
static const u_char PROTO_NVPII     = 11; /* network voice protocol*/
static const u_char PROTO_PUP       = 12; /* pup */
static const u_char PROTO_ARGUS     = 13; /* Argus */
static const u_char PROTO_EMCON     = 14; /* EMCON */
static const u_char PROTO_XNET      = 15; /* Cross Net Debugger */
static const u_char PROTO_CHAOS     = 16; /* Chaos*/
static const u_char PROTO_UDP       = 17; /* user datagram protocol */
static const u_char PROTO_MUX       = 18; /* Multiplexing */
static const u_char PROTO_MEAS      = 19; /* DCN Measurement Subsystems */
static const u_char PROTO_HMP       = 20; /* Host Monitoring */
static const u_char PROTO_PRM       = 21; /* Packet Radio Measurement */
static const u_char PROTO_IDP       = 22; /* xns idp */
static const u_char PROTO_TRUNK1    = 23; /* Trunk-1 */
static const u_char PROTO_TRUNK2    = 24; /* Trunk-2 */
static const u_char PROTO_LEAF1     = 25; /* Leaf-1 */
static const u_char PROTO_LEAF2     = 26; /* Leaf-2 */
static const u_char PROTO_RDP       = 27; /* Reliable Data */
static const u_char PROTO_IRTP      = 28; /* Reliable Transaction */
static const u_char PROTO_TP        = 29; /* tp-4 w/ class negotiation */
static const u_char PROTO_BLT       = 30; /* Bulk Data Transfer */
static const u_char PROTO_NSP       = 31; /* Network Services */
static const u_char PROTO_INP       = 32; /* Merit Internodal */
static const u_char PROTO_SEP       = 33; /* Sequential Exchange */
static const u_char PROTO_3PC       = 34; /* Third Party Connect */
static const u_char PROTO_IDPR      = 35; /* InterDomain Policy Routing */
static const u_char PROTO_XTP       = 36; /* XTP */
static const u_char PROTO_DDP       = 37; /* Datagram Delivery */
static const u_char PROTO_CMTP      = 38; /* Control Message Transport */
static const u_char PROTO_TPXX      = 39; /* TP++ Transport */
static const u_char PROTO_IL        = 40; /* IL transport protocol */
static const u_char PROTO_SIP       = 41; /* Simple Internet Protocol */
static const u_char PROTO_SDRP      = 42; /* Source Demand Routing */
static const u_char PROTO_SIPSR     = 43; /* SIP Source Route */
static const u_char PROTO_SIPFRAG   = 44; /* SIP Fragment */
static const u_char PROTO_IDRP      = 45; /* InterDomain Routing*/
static const u_char PROTO_RSVP      = 46; /* resource reservation */
static const u_char PROTO_GRE       = 47; /* General Routing Encap. */
static const u_char PROTO_MHRP      = 48; /* Mobile Host Routing */
static const u_char PROTO_BHA       = 49; /* BHA */
static const u_char PROTO_ESP       = 50; /* SIPP Encap Sec. Payload */
static const u_char PROTO_AH        = 51; /* SIPP Auth Header */
static const u_char PROTO_INLSP     = 52; /* Integ. Net Layer Security */
static const u_char PROTO_SWIPE     = 53; /* IP with encryption */
static const u_char PROTO_NHRP      = 54; /* Next Hop Resolution */
/* 55-60: Unassigned */
static const u_char PROTO_AHIP      = 61; /* any host internal protocol */
static const u_char PROTO_CFTP      = 62; /* CFTP */
static const u_char PROTO_HELLO     = 63; /* "hello" routing protocol */
static const u_char PROTO_SATEXPAK  = 64; /* SATNET/Backroom EXPAK */
static const u_char PROTO_KRYPTOLAN = 65; /* Kryptolan */
static const u_char PROTO_RVD       = 66; /* Remote Virtual Disk */
static const u_char PROTO_IPPC      = 67; /* Pluribus Packet Core */
static const u_char PROTO_ADFS      = 68; /* Any distributed FS */
static const u_char PROTO_SATMON    = 69; /* Satnet Monitoring */
static const u_char PROTO_VISA      = 70; /* VISA Protocol */
static const u_char PROTO_IPCV      = 71; /* Packet Core Utility */
static const u_char PROTO_CPNX      = 72; /* Comp. Prot. Net. Executive */
static const u_char PROTO_CPHB      = 73; /* Comp. Prot. HeartBeat */
static const u_char PROTO_WSN       = 74; /* Wang Span Network */
static const u_char PROTO_PVP       = 75; /* Packet Video Protocol */
static const u_char PROTO_BRSATMON  = 76; /* BackRoom SATNET Monitoring */
static const u_char PROTO_ND        = 77; /* Sun net disk proto (temp.) */
static const u_char PROTO_WBMON     = 78; /* WIDEBAND Monitoring */
static const u_char PROTO_WBEXPAK   = 79; /* WIDEBAND EXPAK */
static const u_char PROTO_EON       = 80; /* ISO cnlp */
static const u_char PROTO_VMTP      = 81; /* VMTP */
static const u_char PROTO_SVMTP     = 82; /* Secure VMTP */
static const u_char PROTO_VINES     = 83; /* Banyon VINES */
static const u_char PROTO_TTP       = 84; /* TTP */
static const u_char PROTO_IGP       = 85; /* NSFNET-IGP */
static const u_char PROTO_DGP       = 86; /* dissimilar gateway prot. */
static const u_char PROTO_TCF       = 87; /* TCF */
static const u_char PROTO_IGRP      = 88; /* Cisco/GXS IGRP */
static const u_char PROTO_OSPFIGP   = 89; /* OSPFIGP */
static const u_char PROTO_SRPC      = 90; /* Strite RPC protocol */
static const u_char PROTO_LARP      = 91; /* Locus Address Resoloution */
static const u_char PROTO_MTP       = 92; /* Multicast Transport */
static const u_char PROTO_AX25      = 93; /* AX.25 Frames */
static const u_char PROTO_IPEIP     = 94; /* IP encapsulated in IP */
static const u_char PROTO_MICP      = 95; /* Mobile Int.ing control */
static const u_char PROTO_SCCSP     = 96; /* Semaphore Comm. security */
static const u_char PROTO_ETHERIP   = 97; /* Ethernet IP encapsulation */
static const u_char PROTO_ENCAP     = 98; /* encapsulation header */
static const u_char PROTO_APES      = 99; /* any private encr. scheme */
static const u_char PROTO_GMTP     = 100; /* GMTP*/
/* 101-254: Partly Unassigned */
static const u_char PROTO_PGM      = 113; /* PGM */
  u_short ip_sum; /* checksum */
  struct in_addr ip_src; /* source address */
  struct in_addr ip_dst; /* dest address */
};

int Pcap::inspect_ip(const PacketDataUnit& packet)
{
  const ip_header *header = (const ip_header *)packet.data;

  PacketDataUnit ip_data;
  ip_data.time_sent = packet.time_sent;
  ip_data.time_received = packet.time_received;
  ip_data.length = packet.length - header->ip_hl;
  ip_data.data = packet.data + header->ip_hl;

  switch(ntohs(header->ip_p))
  {
    case ip_header::PROTO_ICMP:
      inspect_icmp(ip_data);
      break;

    case ip_header::PROTO_TCP:
      inspect_tcp(ip_data);
      break;

    case ip_header::PROTO_UDP:
      inspect_udp(ip_data);
      break;
  }

  return 0;
}
/*
static const int FRAGMENT_LIFETIME = 30;

struct SPuzzleBuffer
{
  uint32_t m_identification; //!< Fragment identification
  struct timeval m_timestamp;

  char m_buffer[UINT16_MAX]; //!< Buffer containing pcket being assembled
  char* m_data; //!< Pointer to fragmentable part of the packet
  size_t m_reassembled; //!< Number of bytes already assembled
  bool m_length_valid; //!< Whether the m_length value is valid
  size_t m_length; //!< Number of bytes to be assembled

  SPuzzleBuffer()
  {
    m_reassembled = 0;
    m_length_valid = false;
  }
};

typedef std::list<SPuzzleBuffer*> TPuzzleList;

size_t CNetwork::reassemble_ipv4( char* buffer, size_t length )
{
  // check fragmentation
  struct iphdr* ipheader = (struct iphdr*)(buffer + sizeof(struct ethhdr));

  // 0x3fff = 0x1fff (fragmenting offset) + 0x2000 (a "more fragments" flag)
  if(( ntohs(ipheader->frag_off) & 0x3fff ) == 0 )
    return length;

  // length of the IP payload
  size_t payload_length = ntohs(ipheader->tot_len) - 4*ipheader->ihl;
  size_t offset = 8*( ntohs(ipheader->frag_off) & 0x1fff );
  // length of unfragmentable part (ethernet + IP header)
  size_t header_length = sizeof(struct ethhdr) + 4*ipheader->ihl;

  logTrace( LOG_DEBUG, "CNetwork::reassemble_ipv4() header %lu, payload %lu, offset %lu",
    header_length, payload_length, offset );

  SPuzzleBuffer* puzzle = NULL;

  struct timeval timestamp;
  gettimeofday( &timestamp, NULL );

  // search the reassembling buffer
  TPuzzleList::iterator looker = puzzles_ipv4.begin();
  while( looker != puzzles_ipv4.end() )
  {
    TPuzzleList::iterator this_puzzle = looker++;
    // compare source and destination address and fragment identification
    if( memcmp( &ipheader->saddr,
        (*this_puzzle)->m_buffer + sizeof(struct ethhdr) + offsetof(struct iphdr, saddr),
        2*sizeof(struct in_addr) ) == 0 &&
      ipheader->id == (*this_puzzle)->m_identification )
    {
      puzzle = (*this_puzzle);
      break;
    }
    // check expired packets
    if( timestamp.tv_sec - (*this_puzzle)->m_timestamp.tv_sec > FRAGMENT_LIFETIME )
    {
      logTrace( LOG_WARNING, "CNetwork::reassemble_ipv4() packet expired" );
      delete (*this_puzzle);
      puzzles_ipv4.erase( this_puzzle );
    }
  }
  if( puzzle == NULL )
  {
    puzzle = new SPuzzleBuffer();
    NatAssert( puzzle != NULL, CNatException::Crit_CannotAllocateMemory );
    // initialize structure
    puzzle->m_identification = ipheader->id;
    memcpy( puzzle->m_buffer, buffer, header_length );
    puzzle->m_data = puzzle->m_buffer + header_length;
    // put structure into list
    puzzles_ipv4.push_back( puzzle );
  }
  // refresh timestamp
  puzzle->m_timestamp = timestamp;

  // place fragment to the right place
  memcpy( puzzle->m_data + offset, buffer + header_length, payload_length );
  puzzle->m_reassembled += payload_length;
  // this is the last fragment, we can compute overall packet size
  if(( ntohs(ipheader->frag_off) & 0x2000 ) == 0 )
  {
    puzzle->m_length_valid = true;
    puzzle->m_length = offset + payload_length;
  }
  // reassembling is not finished yet
  if( !puzzle->m_length_valid || puzzle->m_reassembled < puzzle->m_length )
  {
    logTrace( LOG_DEBUG, "CNetwork::reassemble_ipv4() fragmented packet is not complete" );
    return 0;
  }

  // create result
  size_t bytes_assembled = ( puzzle->m_data - puzzle->m_buffer ) + puzzle->m_length;
  memcpy( buffer, puzzle->m_buffer, bytes_assembled );
  // the ipheader now contains header of the assembled packet
  ipheader->tot_len = htons( 4*ipheader->ihl + puzzle->m_length );
  ipheader->frag_off = 0;

  puzzles_ipv4.remove( puzzle );
  delete puzzle;

  return bytes_assembled;
}
*/

// $Id: ip.cpp 304 2009-09-10 18:35:08Z gotthardp $
