ADD_LIBRARY(scengmann SHARED
  export.h
  module.cpp
  engmann.h
  engmann.cpp
)

TARGET_LINK_LIBRARIES(scengmann
  scmsc
)

INSTALL(TARGETS scengmann
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
