#include "utils.hpp"

#include "detail/number.h"

class test_number_reader : public fixture_base {};
class test_fixreal       : public fixture_base {};
class test_floreal       : public fixture_base {};

TEST_F (test_number_reader, string_to_number)
{
    EXPECT_TRUE (r_exact_p (r_cstr_to_number (r, "#e1e128")));
    EXPECT_TRUE (r_exact_p (r_cstr_to_number (r, "1/2")));
    EXPECT_TRUE (r_exact_p (r_cstr_to_number (r, "#e1.2")));
}

TEST_F (test_fixreal, r_fixreal_new)
{
    mpq_t value;
    mpq_init (value);

    mpq_set_si (value, 2, 3);
    EXPECT_FALSE (r_failure_p (r_fixreal_new (r, value)));

    mpq_clear (value);
}

TEST_F (test_fixreal, r_fixreal_new_si)
{
    rsexp actual = r_fixreal_new_si (r, 3, 2);

    EXPECT_FALSE (r_failure_p (actual));
    EXPECT_TRUE (mpq_cmp_si (fixreal_from_sexp (actual)->value, 3, 2) == 0);
}

TEST_F (test_floreal, r_floreal_new)
{
    rsexp actual = r_floreal_new (r, 1.5);

    EXPECT_FALSE (r_failure_p (actual));
    EXPECT_TRUE (floreal_from_sexp (actual)->value == 1.5);
}
