# Positive test-cases
ADD_CHECKER_TEST(scorder "Acyclic" acyclic1.mpr 1)
ADD_CHECKER_TEST(scorder "Acyclic" acyclic2.mpr 1)
ADD_CHECKER_TEST(scorder "Acyclic" acyclic3.mpr 1)
ADD_CHECKER_TEST(scorder "Acyclic" acyclic4.mpr 1)
ADD_CHECKER_TEST(scorder "Acyclic" acyclic5.mpr 1)
ADD_CHECKER_TEST(scorder "Acyclic" acyclic6.mpr 1)
# Negative test-cases
ADD_CHECKER_TEST(scorder "Acyclic" cyclic1.mpr 0)
ADD_CHECKER_TEST(scorder "Acyclic" cyclic2.mpr 0)
ADD_CHECKER_TEST(scorder "Acyclic" cyclic3.mpr 0)
ADD_CHECKER_TEST(scorder "Acyclic" cyclic4.mpr 0)
