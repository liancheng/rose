#include "rose/sexp.h"

#include <gtest/gtest.h>

TEST(test_sexp, SEXP_NULL_should_succeed)
{
    ASSERT_TRUE(SEXP_NULL_P(SEXP_NULL));
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
