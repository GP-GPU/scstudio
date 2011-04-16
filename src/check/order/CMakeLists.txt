ADD_LIBRARY(scorder SHARED
  export.h
  module.cpp
  acyclic_checker.cpp
  acyclic_checker.h
  fifo_checker.cpp
  fifo_checker.h
)

TARGET_LINK_LIBRARIES(scorder
  scmsc
  scpseudocode
)

INSTALL(TARGETS scorder
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 151 2009-01-04 19:25:27Z gotthardp $
