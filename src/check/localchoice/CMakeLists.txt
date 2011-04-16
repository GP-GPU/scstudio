ADD_LIBRARY(sclocalchoice SHARED
  export.h
  module.cpp
  local_choice_checker.cpp
  local_choice_checker.h
)

TARGET_LINK_LIBRARIES(sclocalchoice
  scmsc
  scpseudocode
)

INSTALL(TARGETS sclocalchoice
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 750 2010-05-08 16:31:21Z gotthardp $
