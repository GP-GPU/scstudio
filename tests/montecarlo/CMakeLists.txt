ADD_EXECUTABLE(montecarlo_test
  montecarlo_test.cpp
)

TARGET_LINK_LIBRARIES(montecarlo_test
  scmontecarlo
  scmsc
  scZ120
)

GET_TARGET_PROPERTY(MONTECARLO_TEST_EXECUTABLE montecarlo_test LOCATION)
# Replace the "$(IntDir)", "$(OutDir)", or "$(CONFIGURATION)"
# depending on the generator being used with the test-time variable.
STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}"
  MONTECARLO_TEST_EXECUTABLE "${MONTECARLO_TEST_EXECUTABLE}")

ADD_TEST(montecarlo_test-01 ${MONTECARLO_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_simple_bmsc.mpr 0)
ADD_TEST(montecarlo_test-02 ${MONTECARLO_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_cycle_bmsc.mpr 1)
ADD_TEST(montecarlo_test-02 ${MONTECARLO_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_simple_hmsc.mpr 0)
