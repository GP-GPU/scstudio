# do not make antlr a mandatory prerequisite
IF(ANTLR_FOUND)
  ADD_DEFINITIONS(-DHAVE_ANTLR)
  INCLUDE_DIRECTORIES(${ANTLR_INCLUDE_DIR})

  ADD_CUSTOM_COMMAND(
    OUTPUT Z120Lexer.c Z120Lexer.h Z120Parser.c Z120Parser.h
    COMMAND ${JAVA_RUNTIME} -classpath ${ANTLR_CLASSPATH} org.antlr.Tool Z120.g
    DEPENDS Z120.g)

  SET(PARSER_SOURCES
    z120_load.cpp
    Z120.g
    Context.h
    Context_Impl.h
    Context.cpp
    display_error.h
    display_error.cpp
    Z120Lexer.c
    Z120Parser.c)

  SET(PARSER_LIBRARIES
    ${ANTLR_LIBRARIES})
ENDIF(ANTLR_FOUND)

ADD_LIBRARY(scZ120 SHARED
  export.h
  module.cpp
  z120.h
  z120_save.cpp
  ${PARSER_SOURCES}
)

TARGET_LINK_LIBRARIES(scZ120
  scmsc
  ${PARSER_LIBRARIES}
)

INSTALL(TARGETS scZ120
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
