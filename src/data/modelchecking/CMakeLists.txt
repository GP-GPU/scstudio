ADD_LIBRARY(scmodelchecking SHARED
  export.h
  module.cpp
  divine.h
  divine.cpp
)

TARGET_LINK_LIBRARIES(scmodelchecking
  scmsc
  scpseudocode
)

INSTALL(TARGETS scmodelchecking
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
