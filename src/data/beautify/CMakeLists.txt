OPTION(BUILD_OPTIMIZER "Enable to build the layout optimizer" OFF)
IF(BUILD_OPTIMIZER)
  FIND_PACKAGE(LpSolve REQUIRED)
  INCLUDE_DIRECTORIES(${LPSOLVE_INCLUDE_DIR})

  SET(OPTIMIZER_SOURCES
    layout_optimizer.cpp
    layout_optimizer.h
  )
  SET(OPTIMIZER_LIBRARIES
    ${LPSOLVE_LIBRARY}
  )
ENDIF(BUILD_OPTIMIZER)

ADD_LIBRARY(scbeautify SHARED
  export.h
  module.cpp
  beautify.h
  beautify.cpp
  instance_sequencer.h
  instance_sequencer.cpp
  length_optimizer.cpp
  length_optimizer.h
  ${OPTIMIZER_SOURCES}
)

TARGET_LINK_LIBRARIES(scbeautify
  scmsc
  scpseudocode
  ${OPTIMIZER_LIBRARIES}
)

# $Id: CMakeLists.txt 1015 2010-12-27 15:14:09Z xpekarc $
