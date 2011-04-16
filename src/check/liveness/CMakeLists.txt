ADD_LIBRARY(scliveness SHARED
  export.h
  module.cpp
  deadlock_checker.cpp
  deadlock_checker.h
  livelock_checker.cpp
  livelock_checker.h
)

TARGET_LINK_LIBRARIES(scliveness
  scmsc
  scpseudocode
)

INSTALL(TARGETS scliveness
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 165 2009-01-11 19:50:35Z babicaj $
