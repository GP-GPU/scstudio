ADD_LIBRARY(screalizability SHARED
  export.h
  module.cpp
  realizability_checker.cpp
  realizability_checker.h
)

TARGET_LINK_LIBRARIES(screalizability
  scmsc
  scpseudocode
)

INSTALL(TARGETS screalizability
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 750 2010-05-08 16:31:21Z gotthardp $
