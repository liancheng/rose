#include "utils.hpp"

#include "detail/math.h"
#include "detail/number.h"
#include "rose/eq.h"
#include "rose/number.h"

class test_math : public fixture_base {};
class test_addition : public test_math {};

TEST_F (test_math, fixnum_normalization)
{
    {
        rsexp n = smi_to_fixnum (r, R_ONE);
        EXPECT_TRUE (r_fixnum_p (n));
        EXPECT_TRUE (r_small_int_p (r_fixnum_normalize (n)));
    }

    {
        rsexp n = r_add (r, r_int_to_sexp (R_SMI_MAX), R_ONE);
        EXPECT_TRUE (r_fixnum_p (n));
        EXPECT_TRUE (r_fixnum_p (r_fixnum_normalize (n)));
    }
}

TEST_F (test_math, smi_limit)
{
    EXPECT_TRUE (r_small_int_p (r_int_to_sexp (R_SMI_MAX)));
    EXPECT_TRUE (r_small_int_p (r_int_to_sexp (R_SMI_MIN)));
    EXPECT_TRUE (r_fixnum_p (r_add (r, r_int_to_sexp (R_SMI_MAX), R_ONE)));
    EXPECT_TRUE (r_fixnum_p (r_minus (r, r_int_to_sexp (R_SMI_MIN), R_ONE)));
}

TEST_F (test_math, test_add_smi_overflow_p)
{
    EXPECT_TRUE (smi_sum_overflow_p (1, R_SMI_MAX, 1 + R_SMI_MAX));
    EXPECT_TRUE (smi_sum_overflow_p (-1, R_SMI_MIN, -1 + R_SMI_MAX));
}

TEST_F (test_addition, test_add)
{
    EXPECT_TRUE (eq_p (R_ZERO, r_eval_from_cstr (r, "(+)")));
    EXPECT_TRUE (eq_p (R_ONE, r_eval_from_cstr (r, "(+ 1)")));
}
