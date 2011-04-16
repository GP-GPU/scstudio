ADD_LIBRARY(scpseudocode SHARED
  causal_closure_initiator.cpp
  causal_closure_initiator.h
  visual_closure_initiator.cpp
  visual_closure_initiator.h
  utils.cpp
  utils.h
  msc_duplicators.cpp
  msc_duplicators.h
  communication_graph.cpp
  communication_graph.h
)

TARGET_LINK_LIBRARIES(scpseudocode
  scmsc
)

INSTALL(TARGETS scpseudocode
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 522 2009-12-14 18:51:56Z kocianon $
