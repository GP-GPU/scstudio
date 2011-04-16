ADD_LIBRARY(scmontecarlo SHARED
  export.h
  module.cpp
  montecarlo.cpp
  montecarlo.h
)

TARGET_LINK_LIBRARIES(scmontecarlo
  scmsc
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

INSTALL(TARGETS scmontecarlo
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# $Id: CMakeLists.txt 605 2010-02-20 17:01:05Z gotthardp $
