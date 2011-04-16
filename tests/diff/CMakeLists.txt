FIND_PACKAGE(PythonInterp REQUIRED)

ADD_EXECUTABLE(diff_test 
  diff_test.cpp
)

TARGET_LINK_LIBRARIES(diff_test
  scmembership
  scpseudocode
  scmsc
  scZ120
)

#ADD_TEST(diff_test-01 ${DIFF_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/spec01.mpr ${CMAKE_CURRENT_SOURCE_DIR}/flow01.mpr 1)

ADD_DIFF_TEST(spec01.mpr flow01.mpr 1)
ADD_DIFF_TEST(spec02.mpr flow02_1.mpr 1)
ADD_DIFF_TEST(spec02.mpr flow02_2.mpr 1)
ADD_DIFF_TEST(spec02.mpr flow02_3.mpr 1)
ADD_DIFF_TEST(spec02.mpr flow02_4.mpr 1)
ADD_DIFF_TEST(spec02.mpr flow02_5.mpr 1)
ADD_DIFF_TEST(spec02.mpr flow02_6.mpr 1)
ADD_DIFF_TEST(spec02.mpr flow02_7.mpr 1)
ADD_DIFF_TEST(spec03.mpr flow03_1.mpr 1)
ADD_DIFF_TEST(spec03.mpr flow03_2.mpr 1)
ADD_DIFF_TEST(spec04.mpr flow04_1.mpr 1)
ADD_DIFF_TEST(spec04.mpr flow04_2.mpr 1)
ADD_DIFF_TEST(spec04.mpr flow04_3.mpr 1)
ADD_DIFF_TEST(spec04.mpr flow04_4.mpr 1)
ADD_DIFF_TEST(spec04.mpr flow04_5.mpr 1)
ADD_DIFF_TEST(spec04.mpr flow04_6.mpr 1)
ADD_DIFF_TEST(spec05.mpr flow05_1.mpr 1)
ADD_DIFF_TEST(spec05.mpr flow05_2.mpr 1)
ADD_DIFF_TEST(spec05.mpr flow05_3.mpr 1)
ADD_DIFF_TEST(spec06.mpr flow06_1.mpr 1)
ADD_DIFF_TEST(spec06.mpr flow06_2.mpr 1)
ADD_DIFF_TEST(spec06.mpr flow06_3.mpr 1)
ADD_DIFF_TEST(spec07.mpr flow07_1.mpr 1)
ADD_DIFF_TEST(spec07.mpr flow07_2.mpr 1)
ADD_DIFF_TEST(spec08.mpr flow08_1.mpr 1)
ADD_DIFF_TEST(spec09.mpr flow09_1.mpr 1)

