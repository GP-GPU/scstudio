ADD_LIBRARY(scpycheck SHARED
  export.h
  module.cpp
  deadlock_checker_visio.h
  deadlock_checker_visio.cpp
  livelock_checker_visio.h
  livelock_checker_visio.cpp
  acyclic_checker_visio.h
  acyclic_checker_visio.cpp
  fifo_checker_visio.h
  fifo_checker_visio.cpp
  universal_checker_visio.h
  universal_checker_visio.cpp
)

TARGET_LINK_LIBRARIES(scpycheck
  scpyconv
  scmsc
  ${PYTHON_LIBRARIES}
  ${PARSER_LIBRARIES}
)

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

INSTALL(TARGETS scpycheck
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
