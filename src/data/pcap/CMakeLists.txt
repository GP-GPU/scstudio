FIND_PACKAGE(PCAP REQUIRED)

INCLUDE_DIRECTORIES(${PCAP_INCLUDE_DIR})

ADD_LIBRARY(scpcap SHARED
  export.h
  module.cpp
  pcap_load.h
  pcap_load.cpp
  arp.cpp
  ethernet.cpp
  icmp.cpp
  ip.cpp
  tcp.cpp
  udp.cpp
)

TARGET_LINK_LIBRARIES(scpcap
  scmsc
  ${PCAP_LIBRARY}
  ws2_32.lib
)
