FIND_PACKAGE(LATEX)

MACRO(ADD_PDFLATEX_DOCUMENT FILE)
  ADD_CUSTOM_TARGET(${FILE} ALL
    DEPENDS ${FILE}.pdf
  )
  # latex needs to be executed twice in order to generate references correctly
  ADD_CUSTOM_COMMAND(
    OUTPUT ${FILE}.aux
    COMMAND ${PDFLATEX_COMPILER} --interaction batchmode ${FILE}.tex
    DEPENDS ${FILE}.tex
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating ${FILE}.pdf (first pass)"
  )
  ADD_CUSTOM_COMMAND(
    OUTPUT ${FILE}.pdf
    COMMAND ${PDFLATEX_COMPILER} --interaction batchmode ${FILE}.tex
    DEPENDS ${FILE}.tex ${FILE}.aux
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating ${FILE}.pdf (second pass)"
  )
ENDMACRO(ADD_PDFLATEX_DOCUMENT)

IF(PDFLATEX_COMPILER)
  ADD_PDFLATEX_DOCUMENT(beautify)
  ADD_PDFLATEX_DOCUMENT(memb_alg)
ENDIF(PDFLATEX_COMPILER)


ADD_SUBDIRECTORY(help)

# $Id: CMakeLists.txt 379 2009-09-26 12:40:09Z gotthardp $
