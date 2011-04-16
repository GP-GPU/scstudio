ADD_LIBRARY(scstructure SHARED
  export.h
  module.cpp
  name_checker.cpp
  name_checker.h
  nonrecursivity_checker.cpp
  nonrecursivity_checker.h
)

TARGET_LINK_LIBRARIES(scstructure
  scpseudocode
  scmsc
)

INSTALL(TARGETS scstructure
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 750 2010-05-08 16:31:21Z gotthardp $
