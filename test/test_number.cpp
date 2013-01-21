#include "utils.hpp"

#include "detail/finer_number.h"
#include "detail/number.h"
#include "rose/string.h"

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
    mpq_t init;

    mpq_init (init);
    mpq_set_si (init, 6, 4);

    rsexp actual = r_fixreal_new (r, init);
    EXPECT_FALSE (r_failure_p (actual));
    EXPECT_TRUE (r_fixreal_p (actual));
    EXPECT_EQ (0, mpq_cmp_si (fixreal_from_sexp (actual)->value, 3, 2));

    mpq_clear (init);
}

TEST_F (test_fixreal, r_fixreal_new_si)
{
    rsexp actual = r_fixreal_new_si (r, 6, 4);

    EXPECT_FALSE (r_failure_p (actual));
    EXPECT_TRUE (r_fixreal_p (actual));
    EXPECT_EQ (0, mpq_cmp_si (fixreal_from_sexp (actual)->value, 3, 2));
}

TEST_F (test_fixreal, write_fixreal)
{
    rsexp port = r_open_output_string (r);
    r_port_write (r, port, r_fixreal_new_si (r, 6, 4));
    EXPECT_TRUE (equal_p (r_string_new (r, "3/2"),
                          r_get_output_string (r, port)));
}

TEST_F (test_floreal, r_floreal_new)
{
    rsexp actual = r_floreal_new (r, 1.5);

    EXPECT_FALSE (r_failure_p (actual));
    EXPECT_TRUE (r_floreal_p (actual));
    EXPECT_TRUE (floreal_from_sexp (actual)->value == 1.5);
}
