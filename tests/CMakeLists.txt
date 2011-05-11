ENABLE_TESTING()

ADD_EXECUTABLE(py_convert_test
  py_convert_test.cpp)
TARGET_LINK_LIBRARIES(py_convert_test
  scmsc
  scpycheck
)
ADD_TEST(py_convert_test ${EXECUTABLE_OUTPUT_PATH}/py_convert_test)

IF(ANTLR_FOUND)
ADD_EXECUTABLE(py_test
  py_test.cpp)
TARGET_LINK_LIBRARIES(py_test
  scmsc
  scZ120
  scpycheck
)
ADD_TEST(py_test ${EXECUTABLE_OUTPUT_PATH}/py_test)
ENDIF(ANTLR_FOUND)

ADD_EXECUTABLE(universal_boundedness_checker_test
  universal_boundedness_checker_test.cpp
)
TARGET_LINK_LIBRARIES(universal_boundedness_checker_test
  scboundedness
  scmsc
  scpseudocode
)
ADD_TEST(universal_boundedness_checker ${EXECUTABLE_OUTPUT_PATH}/universal_boundedness_checker_test)

ADD_EXECUTABLE(local_choice_checker_test
  local_choice_checker_test.cpp
)
TARGET_LINK_LIBRARIES(local_choice_checker_test
  sclocalchoice
  scmsc
  scpseudocode
)
ADD_TEST(local_choice_checker
 ${EXECUTABLE_OUTPUT_PATH}/local_choice_checker_test)


ADD_EXECUTABLE(deadlock_checker_test
  deadlock_checker_test.cpp
)
TARGET_LINK_LIBRARIES(deadlock_checker_test
  scliveness
  scmsc
)
ADD_TEST(deadlock_checker ${EXECUTABLE_OUTPUT_PATH}/deadlock_checker_test)

ADD_EXECUTABLE(livelock_checker_test
  livelock_checker_test.cpp
)
TARGET_LINK_LIBRARIES(livelock_checker_test
  scliveness
  scmsc
)
ADD_TEST(livelock_checker_checker ${EXECUTABLE_OUTPUT_PATH}/livelock_checker_test)

ADD_EXECUTABLE(fifo_checker_test
  fifo_checker_test.cpp
)
TARGET_LINK_LIBRARIES(fifo_checker_test
  scorder
  scmsc
  scpseudocode
)
ADD_TEST(fifo_checker ${EXECUTABLE_OUTPUT_PATH}/fifo_checker_test)

ADD_EXECUTABLE(acyclic_checker_test
  acyclic_checker_test.cpp
)
TARGET_LINK_LIBRARIES(acyclic_checker_test
  scorder
  scmsc
  sctime
  scpseudocode
)
ADD_TEST(acyclic_checker ${EXECUTABLE_OUTPUT_PATH}/acyclic_checker_test)



ADD_EXECUTABLE(find_block_test
  find_block_test.cpp
)
TARGET_LINK_LIBRARIES(find_block_test
  scmsc
  sctime
  scpseudocode
)
ADD_TEST(find_block ${EXECUTABLE_OUTPUT_PATH}/find_block_test)



ADD_EXECUTABLE(race_checker_test
  race_checker_test.cpp
)
TARGET_LINK_LIBRARIES(race_checker_test
  scrace
  scmsc
  scpseudocode
)
ADD_TEST(race_checker ${EXECUTABLE_OUTPUT_PATH}/race_checker_test)

ADD_EXECUTABLE(engmann_test
  engmann_test.cpp
)
TARGET_LINK_LIBRARIES(engmann_test
  scengmann
  scZ120
)
ADD_TEST(engmann ${EXECUTABLE_OUTPUT_PATH}/engmann_test engmann_test01.cfi)

IF(ANTLR_FOUND)
ADD_DEFINITIONS(-DHAVE_ANTLR)

SET(Z120_SEQUENCE 0)
MACRO(ADD_Z120_TEST FILE SATISFIED)
  GET_TARGET_PROPERTY(Z120_TEST_EXECUTABLE z120_test LOCATION)
  # Replace the "$(IntDir)", "$(OutDir)", or "$(CONFIGURATION)"
  # depending on the generator being used with the test-time variable.
  STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}"
    Z120_TEST_EXECUTABLE "${Z120_TEST_EXECUTABLE}")

  ADD_TEST("z120_test-${Z120_SEQUENCE}-${FILE}"
    ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tests/z120_test/z120_diff.py" "${Z120_TEST_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}" ${SATISFIED})
  MATH(EXPR Z120_SEQUENCE ${Z120_SEQUENCE}+1)
ENDMACRO(ADD_Z120_TEST)

ADD_SUBDIRECTORY(z120_test)

ADD_SUBDIRECTORY(membership)

SET(DIFF_SEQUENCE 0)
MACRO(ADD_DIFF_TEST FILE1 FILE2 SATISFIED)
  GET_TARGET_PROPERTY(DIFF_TEST_EXECUTABLE diff_test LOCATION)
  # Replace the "$(IntDir)", "$(OutDir)", or "$(CONFIGURATION)"
  # depending on the generator being used with the test-time variable.
  STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}"
    DIFF_TEST_EXECUTABLE "${DIFF_TEST_EXECUTABLE}")

  ADD_TEST("diff_test-${DIFF_SEQUENCE}-${FILE2}"
    ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tests/diff/diff_diff.py" "${DIFF_TEST_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/${FILE1}" "${CMAKE_CURRENT_SOURCE_DIR}/${FILE2}" ${SATISFIED})
  MATH(EXPR DIFF_SEQUENCE ${DIFF_SEQUENCE}+1)
ENDMACRO(ADD_DIFF_TEST)

ADD_SUBDIRECTORY(diff)

ADD_SUBDIRECTORY(montecarlo)

ADD_EXECUTABLE(checker_test
  checker_test.cpp
)
TARGET_LINK_LIBRARIES(checker_test
  scZ120
  ${CMAKE_DL_LIBS}
)
GET_TARGET_PROPERTY(CHECKER_TEST_EXECUTABLE checker_test LOCATION)
# Replace the "$(IntDir)", "$(OutDir)", or "$(CONFIGURATION)"
# depending on the generator being used with the test-time variable.
STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}"
  CHECKER_TEST_EXECUTABLE "${CHECKER_TEST_EXECUTABLE}")

SET(CHECKER_SEQUENCE 0)
MACRO(ADD_CHECKER_TEST LIBRARY PROPERTY FILE SATISFIED)
  GET_TARGET_PROPERTY(LIBRARY_LOCATION ${LIBRARY} LOCATION)
  # Replace the "$(IntDir)", "$(OutDir)", or "$(CONFIGURATION)"
  STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}"
    LIBRARY_LOCATION "${LIBRARY_LOCATION}")
  ADD_TEST("checker_test-${CHECKER_SEQUENCE}-${FILE}"
    ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/tests/checker_diff.py" ${CHECKER_TEST_EXECUTABLE} ${LIBRARY_LOCATION} ${PROPERTY} ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} ${SATISFIED})
  MATH(EXPR CHECKER_SEQUENCE ${CHECKER_SEQUENCE}+1)
ENDMACRO(ADD_CHECKER_TEST)

ADD_SUBDIRECTORY(acyclic)
ADD_SUBDIRECTORY(deadlock)
ADD_SUBDIRECTORY(fifo)
ADD_SUBDIRECTORY(race)
ADD_SUBDIRECTORY(local_choice)
ADD_SUBDIRECTORY(universal_boundedness)
ADD_SUBDIRECTORY(time)
ADD_SUBDIRECTORY(time_constraints)

ENDIF(ANTLR_FOUND)

ADD_EXECUTABLE(decimal_test
  decimal_test.cpp
)
TARGET_LINK_LIBRARIES(decimal_test
  scmsc
)

ADD_EXECUTABLE(interval_test
  interval_test.cpp
)
TARGET_LINK_LIBRARIES(interval_test
  scmsc
)

ADD_EXECUTABLE(interval_set_test
  interval_set_test.cpp
)
TARGET_LINK_LIBRARIES(interval_set_test
  scmsc
)

ADD_EXECUTABLE(interval_string
  interval_string.cpp
)
TARGET_LINK_LIBRARIES(interval_string
  scmsc
)

ADD_TEST(decimal_test ${EXECUTABLE_OUTPUT_PATH}/decimal_test)
ADD_TEST(interval_test ${EXECUTABLE_OUTPUT_PATH}/interval_test)
ADD_TEST(interval_set_test ${EXECUTABLE_OUTPUT_PATH}/interval_set_test)
ADD_TEST(interval_string ${EXECUTABLE_OUTPUT_PATH}/interval_string)

#ADD_EXECUTABLE(bmsc_matrix_converter_test
#  bmsc_matrix_converter_test.cpp
#)
#TARGET_LINK_LIBRARIES(bmsc_matrix_converter_test
#  scmsc
#  sctime
#  scpseudocode
#)
#
# ADD_EXECUTABLE(incon_test
#   incon_test.cpp
# )
# TARGET_LINK_LIBRARIES(incon_test
#   scmsc
#   sctime
#   scpseudocode
# )

IF(ANTLR_FOUND)
ADD_EXECUTABLE(tighten_msc_test
  tighten_msc_test.cpp
)
TARGET_LINK_LIBRARIES(tighten_msc_test
  scmsc
  sctime
  scpseudocode
  scZ120
)
ENDIF(ANTLR_FOUND)

IF(ANTLR_FOUND)
ADD_EXECUTABLE(tighten_hmsc_test
  tighten_hmsc_test.cpp
)
TARGET_LINK_LIBRARIES(tighten_hmsc_test
  scmsc
  sctime
  scpseudocode
  scZ120
)
ENDIF(ANTLR_FOUND)

#ADD_EXECUTABLE(duplicator_test

#  duplicator_test.cpp
#)
#TARGET_LINK_LIBRARIES(duplicator_test
#  scmsc
#  sctime
#  scpseudocode
#  scZ120
#)
#
# ADD_EXECUTABLE(bmsc_tightening_test
#   bmsc_tightening_test.cpp
# )
# TARGET_LINK_LIBRARIES(bmsc_tightening_test
#   scmsc
#   sctime
#   scpseudocode
# )

#ADD_EXECUTABLE(constrain_check_test
#  constrain_check_test.cpp
#)

#TARGET_LINK_LIBRARIES(constrain_check_test
#  scmsc
#  sctime
#  scpseudocode
#)

#
#ADD_EXECUTABLE(max_tightener_test
#  max_tightener_test.cpp
#)

#TARGET_LINK_LIBRARIES(max_tightener_test
#  scmsc
#  sctime
#  scpseudocode
#)


#ADD_EXECUTABLE(hmsc_all_paths_test
#  hmsc_all_paths_test.cpp
#)

#TARGET_LINK_LIBRARIES(hmsc_all_paths_test
#  scmsc
#  sctime
#  scpseudocode
#)

#ADD_EXECUTABLE(hmsc_tighten
#  hmsc_all_paths_test.cpp
#)

# $Id: CMakeLists.txt 1039 2011-02-15 17:15:11Z madzin $
