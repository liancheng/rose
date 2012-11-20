#include <gmpxx.h>

extern "C" {

#include "rose/number.h"

}

#include <gtest/gtest.h>

TEST (test_small_int, smi_limit)
{
    EXPECT_TRUE (r_small_int_p (r_int_to_sexp (R_SMI_MAX)));
    EXPECT_TRUE (r_small_int_p (r_int_to_sexp (R_SMI_MIN)));
}
