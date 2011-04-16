ADD_LIBRARY(scboundedness SHARED
  export.h
  module.cpp
  universal_boundedness_checker.cpp
  universal_boundedness_checker.h
)

TARGET_LINK_LIBRARIES(scboundedness
  scmsc
  scpseudocode
)

INSTALL(TARGETS scboundedness
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 206 2009-03-25 17:03:43Z vacek $
