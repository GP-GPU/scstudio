# Positive test-cases
ADD_CHECKER_TEST(scorder "FIFO" fifo1.mpr 1)
ADD_CHECKER_TEST(scorder "FIFO" fifo2.mpr 1)
ADD_CHECKER_TEST(scorder "FIFO" fifo3.mpr 1)
ADD_CHECKER_TEST(scorder "FIFO" fifo4.mpr 1)
ADD_CHECKER_TEST(scorder "FIFO" fifo5.mpr 1)
ADD_CHECKER_TEST(scorder "FIFO" fifo6.mpr 1)
ADD_CHECKER_TEST(scorder "FIFO" fifo7.mpr 1)
# Negative test-cases
ADD_CHECKER_TEST(scorder "FIFO" nonfifo1.mpr 0)
ADD_CHECKER_TEST(scorder "FIFO" nonfifo2.mpr 0)
