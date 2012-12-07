#include "utils.hpp"

#include "rose/number.h"

class test_number_reader : public fixture_base {};

TEST (test_small_int, smi_limit)
{
    EXPECT_TRUE (r_small_int_p (r_int_to_sexp (R_SMI_MAX)));
    EXPECT_TRUE (r_small_int_p (r_int_to_sexp (R_SMI_MIN)));
}

TEST_F (test_number_reader, string_to_number)
{
    EXPECT_TRUE (r_exact_p (r_string_to_number (state, "#e1e128")));
    EXPECT_TRUE (r_exact_p (r_string_to_number (state, "1/2")));
    EXPECT_TRUE (r_exact_p (r_string_to_number (state, "#e1.2")));
}
