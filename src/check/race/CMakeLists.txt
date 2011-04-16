ADD_LIBRARY(scrace SHARED
  export.h
  module.cpp
  race_checker.cpp
  race_checker.h
  footprint.cpp
  footprint.h
)

TARGET_LINK_LIBRARIES(scrace
  scmsc
  scpseudocode
)

INSTALL(TARGETS scrace
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 151 2009-01-04 19:25:27Z gotthardp $
