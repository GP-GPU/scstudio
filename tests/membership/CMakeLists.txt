ADD_EXECUTABLE(membership_test 
  membership_test.cpp
)

TARGET_LINK_LIBRARIES(membership_test
  scmembership
  scpseudocode
  scmsc
  scZ120
)

#ADD_MEMBERSHIP_TEST(sctime "Correct Time Constraint Syntax" connector_correct0.mpr 1)

#ADD_MEMBERSHIP_TEST(test_hmsc01 test_bmsc01 1)

GET_TARGET_PROPERTY(MEMBERSHIP_TEST_EXECUTABLE membership_test LOCATION)
# Replace the "$(IntDir)", "$(OutDir)", or "$(CONFIGURATION)"
# depending on the generator being used with the test-time variable.
STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}"
  MEMBERSHIP_TEST_EXECUTABLE "${MEMBERSHIP_TEST_EXECUTABLE}")

ADD_TEST(membership_test-01 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc01.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc01.mpr 1)

ADD_TEST(membership_test-02 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc02.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc02.mpr 0)

ADD_TEST(membership_test-03 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc03.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc03.mpr 1)

ADD_TEST(membership_test-04 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc04.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc04.mpr 0)

ADD_TEST(membership_test-05 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc05.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc05.mpr 1)

ADD_TEST(membership_test-06 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc06.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc06.mpr 0)

#coregion tests
ADD_TEST(membership_test-08 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc08.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc08.mpr 1)

ADD_TEST(membership_test-09 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc09.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc09.mpr 1)

ADD_TEST(membership_test-10 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc10.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc10.mpr 1)

ADD_TEST(membership_test-11 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc11.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc11_1.mpr 1)

ADD_TEST(membership_test-12 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc11.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc11_2.mpr 0)

ADD_TEST(membership_test-13 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc11.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc11_3.mpr 0)

ADD_TEST(membership_test-14 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc11.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc11_4.mpr 0)

ADD_TEST(membership_test-15 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc12.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc12_1.mpr 1)

ADD_TEST(membership_test-16 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc12.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc12_2.mpr 1)

ADD_TEST(membership_test-17 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc12.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc12_3.mpr 0)

ADD_TEST(membership_test-18 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc12.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc12_4.mpr 0)

ADD_TEST(membership_test-19 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc12.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc12_5.mpr 0)

ADD_TEST(membership_test-20 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc13.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc13_1.mpr 1)

ADD_TEST(membership_test-21 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc13.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc13_2.mpr 1)

ADD_TEST(membership_test-22 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc13.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc13_3.mpr 0)

ADD_TEST(membership_test-23 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc14.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc14_1.mpr 1)

ADD_TEST(membership_test-24 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc14.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc14_2.mpr 0)

ADD_TEST(membership_test-25 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc14.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc14_3.mpr 0)

ADD_TEST(membership_test-26 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc14.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc14_4.mpr 0)

ADD_TEST(membership_test-27 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc15.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc15_1.mpr 1)

ADD_TEST(membership_test-28 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc15.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc15_2.mpr 0)

ADD_TEST(membership_test-29 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc16.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc16_1.mpr 1)

ADD_TEST(membership_test-30 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc16.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc16_2.mpr 1)

ADD_TEST(membership_test-31 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc16.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc16_3.mpr 0)

ADD_TEST(membership_test-32 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc11.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc11_5.mpr 1)

ADD_TEST(membership_test-33 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc17.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc17_1.mpr 1)

ADD_TEST(membership_test-34 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc17.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc17_2.mpr 0)

ADD_TEST(membership_test-35 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc18.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc18_1.mpr 1)

ADD_TEST(membership_test-36 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc18.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc18_2.mpr 0)

ADD_TEST(membership_test-37 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc18.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc18_3.mpr 0)

#due to duplicator, membership works only over MSC graph
#ADD_TEST(membership_test-38 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc19.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc19.mpr 1)
ADD_TEST(membership_test-38 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc19.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc19.mpr 0)

#test bMSC x bMSC 
ADD_TEST(membership_test-39 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc19.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc19.mpr 1)

ADD_TEST(membership_test-40 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc12_1.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc12_1.mpr 1)

ADD_TEST(membership_test-41 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc13_1.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc13_1.mpr 1)

ADD_TEST(membership_test-42 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc15_1.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc15_1.mpr 1)

#rozcvicka
ADD_TEST(membership_test-43 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/rozcvicka_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/rozcvicka_bmsc.mpr 1)

ADD_TEST(membership_test-44 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/rozcvicka_long.mpr ${CMAKE_CURRENT_SOURCE_DIR}/rozcvicka_bmsc.mpr 1)

#test coregion ordering skontrolovat
ADD_TEST(membership_test-45 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc20.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc20_1.mpr 1)

ADD_TEST(membership_test-46 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc20.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc20_2.mpr 0)

#incomplete messages
ADD_TEST(membership_test-47 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc21.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc21_1.mpr 1)

ADD_TEST(membership_test-48 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc21.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc21_2.mpr 1)

ADD_TEST(membership_test-49 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc21.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc21_3.mpr 0)

ADD_TEST(membership_test-50 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc21.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc21_4.mpr 0)

ADD_TEST(membership_test-51 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc22.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc22_1.mpr 1)

ADD_TEST(membership_test-52 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc22.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc22_2.mpr 1)

ADD_TEST(membership_test-53 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc22.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc22_3.mpr 1)

ADD_TEST(membership_test-54 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc22.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc22_4.mpr 1)

ADD_TEST(membership_test-55 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc23.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc23_1.mpr 1)

ADD_TEST(membership_test-56 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc23.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc23_2.mpr 1)

ADD_TEST(membership_test-57 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc23.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc23_3.mpr 1)

ADD_TEST(membership_test-58 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc23.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc23_4.mpr 1)

#empty MSC
ADD_TEST(membership_test-59 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc24.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc24_1.mpr 1)

ADD_TEST(membership_test-60 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc24.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc24_2.mpr 1)

ADD_TEST(membership_test-61 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc25.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc25_1.mpr 1)

ADD_TEST(membership_test-62 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc25.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc25_2.mpr 1)

#not empty
ADD_TEST(membership_test-63 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc26.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc26_1.mpr 1)

ADD_TEST(membership_test-64 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc26.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc26_2.mpr 1)

#bMSC x bMSC with coregion
ADD_TEST(membership_test-65 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc27.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc27_1.mpr 1)

#Lubos' tests
ADD_TEST(membership_test-66 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cycle_neg_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/cycle_neg_pat.mpr 0)

ADD_TEST(membership_test-67 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cycle_pos_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/cycle_pos_pat.mpr 1)

ADD_TEST(membership_test-68 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cycleWithEmpty_pos_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/cycleWithEmpty_pos_pat.mpr 1)

ADD_TEST(membership_test-69 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/hard_neg_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/hard_neg_pat.mpr 0)

ADD_TEST(membership_test-70 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/hard_pos_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/hard_pos_pat.mpr 1)

#Lubos' time tests
ADD_TEST(membership_test-71 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_simple1_pos_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_simple1_pos_pat.mpr 1)

ADD_TEST(membership_test-72 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_simple1_neg_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_simple1_neg_pat.mpr 0)

ADD_TEST(membership_test-73 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_simple2_pos_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_simple2_pos_pat.mpr 1)

ADD_TEST(membership_test-74 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_simple2_neg_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_simple2_neg_pat.mpr 0)

ADD_TEST(membership_test-75 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_cycle_neg_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_cycle_neg_pat.mpr 0)

ADD_TEST(membership_test-76 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_cycle_pos_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_cycle_pos_pat.mpr 1)

ADD_TEST(membership_test-77 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_cycleWithEmpty_pos_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_cycleWithEmpty_pos_pat.mpr 1)

ADD_TEST(membership_test-78 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_cycleWithEmpty_neg_hmsc.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_cycleWithEmpty_neg_pat.mpr 0)

ADD_TEST(membership_test-79 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc28.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc28_1.mpr 1)

ADD_TEST(membership_test-80 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc28.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc28_2.mpr 1)

ADD_TEST(membership_test-81 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc28.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc28_3.mpr 1) # + warning

ADD_TEST(membership_test-82 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc28.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc28_4.mpr 1) # + warning

#tests for new time algorithm 
ADD_TEST(membership_test-83 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc29.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc29_1.mpr 1)

ADD_TEST(membership_test-84 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc29.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc29_2.mpr 1) # + warning

ADD_TEST(membership_test-85 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc30.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc30_1.mpr 1)

ADD_TEST(membership_test-86 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc30.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc30_2.mpr 1) # + warning

ADD_TEST(membership_test-87 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc31.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc31_1.mpr 1)

ADD_TEST(membership_test-88 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc01.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc_pat01.mpr 1)

ADD_TEST(membership_test-89 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc02.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc_pat01.mpr 1) # + warning

ADD_TEST(membership_test-90 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc03.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc_pat01.mpr 0)

ADD_TEST(membership_test-91 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc04.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc_pat02.mpr 1) # + warning

ADD_TEST(membership_test-92 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc05.mpr ${CMAKE_CURRENT_SOURCE_DIR}/time_hard_bmsc_pat02.mpr 0)

#coregion minimal
ADD_TEST(membership_test-93 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal.mpr ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal_pat1.mpr 0)

ADD_TEST(membership_test-94 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal.mpr ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal_pat2.mpr 0)

ADD_TEST(membership_test-95 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal.mpr ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal_pat3.mpr 0)

ADD_TEST(membership_test-96 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal.mpr ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal_pat4.mpr 0)

ADD_TEST(membership_test-97 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal.mpr ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal_pat5.mpr 1)

ADD_TEST(membership_test-98 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal.mpr ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal_pat6.mpr 1)

ADD_TEST(membership_test-99 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal.mpr ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal_pat7.mpr 1)

ADD_TEST(membership_test-100 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal.mpr ${CMAKE_CURRENT_SOURCE_DIR}/coregion_minimal_pat8.mpr 1)

#partial membership
ADD_TEST(membership_test-101 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/partial_mem_easy.mpr ${CMAKE_CURRENT_SOURCE_DIR}/partial_mem_easy_pat1.mpr 1 A)

ADD_TEST(membership_test-102 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/partial_mem_easy.mpr ${CMAKE_CURRENT_SOURCE_DIR}/partial_mem_easy_pat1.mpr 0 A B)

ADD_TEST(membership_test-103 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/partial_mem_easy.mpr ${CMAKE_CURRENT_SOURCE_DIR}/partial_mem_easy.mpr 1 A B)

ADD_TEST(membership_test-104 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/partial_mem_easy.mpr ${CMAKE_CURRENT_SOURCE_DIR}/partial_mem_easy.mpr 1 A B C)




ADD_TEST(membership_test-105 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/complete.mpr ${CMAKE_CURRENT_SOURCE_DIR}/complete_pat1.mpr 0)

ADD_TEST(membership_test-106 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/complete.mpr ${CMAKE_CURRENT_SOURCE_DIR}/complete_pat5.mpr 1)

ADD_TEST(membership_test-107 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/complete.mpr ${CMAKE_CURRENT_SOURCE_DIR}/complete_pat2.mpr 0)

ADD_TEST(membership_test-108 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/complete1.mpr ${CMAKE_CURRENT_SOURCE_DIR}/complete_pat3.mpr 1)

ADD_TEST(membership_test-109 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/complete.mpr ${CMAKE_CURRENT_SOURCE_DIR}/complete_pat1.mpr 1 PC)

ADD_TEST(membership_test-110 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/complete.mpr ${CMAKE_CURRENT_SOURCE_DIR}/complete_pat3.mpr 1 PC Router Server)


#complete.mpr separeted into small pieces
ADD_TEST(membership_test-115 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc32.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc32_1.mpr 1)

ADD_TEST(membership_test-116 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc33.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc33_1.mpr 1)

ADD_TEST(membership_test-117 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc33.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc33_1.mpr 1)

#do not meet preconditions
#ADD_TEST(membership_test-118 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc32.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc32_2.mpr 0)


ADD_TEST(membership_test-119 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_hmsc35.mpr ${CMAKE_CURRENT_SOURCE_DIR}/test_bmsc35.mpr 1)

ADD_TEST(membership_test-120 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/absolut_time_hmsc01.mpr ${CMAKE_CURRENT_SOURCE_DIR}/absolut_time_bmsc01.mpr 1)

ADD_TEST(membership_test-121 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/absolut_time_hmsc02.mpr ${CMAKE_CURRENT_SOURCE_DIR}/absolut_time_bmsc02_1.mpr 1)

ADD_TEST(membership_test-122 ${MEMBERSHIP_TEST_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/absolut_time_hmsc02.mpr ${CMAKE_CURRENT_SOURCE_DIR}/absolut_time_bmsc02_1.mpr 0)
