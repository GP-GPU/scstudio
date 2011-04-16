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
 * $Id: icmp.cpp 304 2009-09-10 18:35:08Z gotthardp $
 */

#include "pcap_load.h"

struct icmp_header
{
  u_char icmp_type; /* type of message, see below */
  u_char icmp_code; /* type sub code */
static const u_char ICMP_ECHOREPLY         = 0; /* echo reply */
static const u_char ICMP_UNREACH           = 3; /* dest unreachable, codes: */
static const u_char   ICMP_UNREACH_NET               = 0; /* bad net */
static const u_char   ICMP_UNREACH_HOST              = 1; /* bad host */
static const u_char   ICMP_UNREACH_PROTOCOL          = 2; /* bad protocol */
static const u_char   ICMP_UNREACH_PORT              = 3; /* bad port */
static const u_char   ICMP_UNREACH_NEEDFRAG          = 4; /* IP_DF caused drop */
static const u_char   ICMP_UNREACH_SRCFAIL           = 5; /* src route failed */
static const u_char   ICMP_UNREACH_NET_UNKNOWN       = 6; /* unknown net */
static const u_char   ICMP_UNREACH_HOST_UNKNOWN      = 7; /* unknown host */
static const u_char   ICMP_UNREACH_ISOLATED          = 8; /* src host isolated */
static const u_char   ICMP_UNREACH_NET_PROHIB        = 9; /* prohibited access */
static const u_char   ICMP_UNREACH_HOST_PROHIB       = 10; /* ditto */
static const u_char   ICMP_UNREACH_TOSNET            = 11; /* bad tos for net */
static const u_char   ICMP_UNREACH_TOSHOST           = 12; /* bad tos for host */
static const u_char   ICMP_UNREACH_FILTER_PROHIB     = 13; /* admin prohib */
static const u_char   ICMP_UNREACH_HOST_PRECEDENCE   = 14; /* host prec vio. */
static const u_char   ICMP_UNREACH_PRECEDENCE_CUTOFF = 15; /* prec cutoff */
static const u_char ICMP_SOURCEQUENCH      = 4; /* packet lost, slow down */
static const u_char ICMP_REDIRECT          = 5; /* shorter route, codes: */
static const u_char   ICMP_REDIRECT_NET        = 0; /* for network */
static const u_char   ICMP_REDIRECT_HOST       = 1; /* for host */
static const u_char   ICMP_REDIRECT_TOSNET     = 2; /* for tos and net */
static const u_char   ICMP_REDIRECT_TOSHOST    = 3; /* for tos and host */
static const u_char ICMP_ALTHOSTADDR       = 6; /* alternate host address */
static const u_char ICMP_ECHO              = 8; /* echo service */
static const u_char ICMP_ROUTERADVERT      = 9; /* router advertisement */
static const u_char   ICMP_ROUTERADVERT_NORMAL         = 0; /* normal advertisement */
static const u_char   ICMP_ROUTERADVERT_NOROUTE_COMMON = 16; /* selective routing */
static const u_char ICMP_ROUTERSOLICIT     = 10; /* router solicitation */
static const u_char ICMP_TIMXCEED          = 11; /* time exceeded, code: */
static const u_char   ICMP_TIMXCEED_INTRANS    = 0; /* ttl==0 in transit */
static const u_char   ICMP_TIMXCEED_REASS      = 1; /* ttl==0 in reass */
static const u_char ICMP_PARAMPROB         = 12; /* ip header bad */
static const u_char   ICMP_PARAMPROB_ERRATPTR  = 0; /* error at param ptr */
static const u_char   ICMP_PARAMPROB_OPTABSENT = 1; /* req. opt. absent */
static const u_char   ICMP_PARAMPROB_LENGTH    = 2; /* bad length */
static const u_char ICMP_TSTAMP            = 13; /* timestamp request */
static const u_char ICMP_TSTAMPREPLY       = 14; /* timestamp reply */
static const u_char ICMP_IREQ              = 15; /* information request */
static const u_char ICMP_IREQREPLY         = 16; /* information reply */
static const u_char ICMP_MASKREQ           = 17; /* address mask request */
static const u_char ICMP_MASKREPLY         = 18; /* address mask reply */
static const u_char ICMP_TRACEROUTE        = 30; /* traceroute */
static const u_char ICMP_DATACONVERR       = 31; /* data conversion error */
static const u_char ICMP_MOBILE_REDIRECT   = 32; /* mobile host redirect */
static const u_char ICMP_IPV6_WHEREAREYOU  = 33; /* IPv6 where-are-you */
static const u_char ICMP_IPV6_IAMHERE      = 34; /* IPv6 i-am-here */
static const u_char ICMP_MOBILE_REGREQUEST = 35; /* mobile registration req */
static const u_char ICMP_MOBILE_REGREPLY   = 36; /* mobile registreation reply */
static const u_char ICMP_SKIP              = 39; /* SKIP */
static const u_char ICMP_PHOTURIS          = 40; /* Photuris */
  u_short icmp_cksum; /* ones complement cksum of struct */
  union
  {
    u_char ih_pptr; /* ICMP_PARAMPROB */
    struct in_addr ih_gwaddr; /* ICMP_REDIRECT */
    struct ih_idseq
    {
      u_short	icd_id;
      u_short	icd_seq;
    } ih_idseq;
    int ih_void;

    /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
    struct ih_pmtu
    {
      u_short ipm_void;
      u_short ipm_nextmtu;
    } ih_pmtu;

    struct ih_rtradv
    {
      u_char irt_num_addrs;
      u_char irt_wpa;
      uint16_t irt_lifetime;
    } ih_rtradv;
  } icmp_hun;
  union
  {
    struct id_ts
    {
      uint32_t its_otime;
      uint32_t its_rtime;
      uint32_t its_ttime;
    } id_ts;
    struct icmp_ra_addr
    {
      uint32_t ira_addr;
      uint32_t ira_preference;
    } id_radv;
    uint32_t id_mask;
    char id_data[1];
  } icmp_dun;
};

int Pcap::inspect_icmp(const PacketDataUnit& packet)
{
  const icmp_header *header = (const icmp_header *)packet.data;

  switch(ntohs(header->icmp_type))
  {
    case icmp_header::ICMP_ECHOREPLY:
      break;
    case icmp_header::ICMP_ECHO:
      break;

    default:
      break;
  }

  return 0;
}

// $Id: icmp.cpp 304 2009-09-10 18:35:08Z gotthardp $
