ADD_SUBDIRECTORY(consistency)
ADD_SUBDIRECTORY(time_race)

ADD_CHECKER_TEST(sctime "Correct Time Constraint Syntax" connector_correct0.mpr 1)
ADD_CHECKER_TEST(sctime "Correct Time Constraint Syntax" connector_correct1.mpr 1)

ADD_CHECKER_TEST(sctime "Correct Time Constraint Syntax" connector_incorrect1.mpr 0)
ADD_CHECKER_TEST(sctime "Correct Time Constraint Syntax" connector_incorrect2.mpr 0)
