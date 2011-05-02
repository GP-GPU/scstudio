ADD_LIBRARY(scpyconv SHARED
  export.h
  py_conv.h
  py_conv.cpp
)

TARGET_LINK_LIBRARIES(scpyconv
  scmsc
  ${PYTHON_LIBRARIES}
  ${PARSER_LIBRARIES}
)

ADD_LIBRARY(scconvpy SHARED
  export.h
  conv_py.h
  conv_py.cpp
)
TARGET_LINK_LIBRARIES(scconvpy
  scmsc
  ${PYTHON_LIBRARIES}
  ${PARSER_LIBRARIES}
)

INSTALL(TARGETS scpyconv
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
