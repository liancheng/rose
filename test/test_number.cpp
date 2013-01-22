#include "utils.hpp"

#include "detail/finer_number.h"
#include "detail/number.h"
#include "rose/string.h"

class test_fixreal : public fixture_base {};
class test_floreal : public fixture_base {};
class test_complex : public fixture_base {};
class test_number  : public fixture_base {};

TEST_F (test_fixreal, r_fixreal_new)
{
    mpq_t init;

    mpq_init (init);
    mpq_set_si (init, 6, 4);

    rsexp actual = r_fixreal_new (r, init);
    EXPECT_FALSE (r_failure_p (actual));
    EXPECT_TRUE (r_fixreal_p (actual));
    EXPECT_EQ (0, mpq_cmp_si (fixreal_from_sexp (actual)->value, 3, 2));

    allocator_disruptor md;
    actual = r_fixreal_new (r, init);
    EXPECT_TRUE (r_failure_p (actual));

    mpq_clear (init);
}

TEST_F (test_fixreal, r_fixreal_new_si)
{
    rsexp actual;

    actual = r_fixreal_new_si (r, 6, 4);
    EXPECT_FALSE (r_failure_p (actual));
    EXPECT_TRUE (r_fixreal_p (actual));
    EXPECT_EQ (0, mpq_cmp_si (fixreal_from_sexp (actual)->value, 3, 2));

    allocator_disruptor md;
    actual = r_fixreal_new_si (r, 6, 4);
    EXPECT_TRUE (r_failure_p (actual));
}

TEST_F (test_fixreal, write_fixreal)
{
    rsexp port = r_open_output_string (r);

    r_port_write (r, port, r_fixreal_new_si (r, 6, 4));
    EXPECT_TRUE (equal_p (r_string_new (r, "3/2"),
                          r_get_output_string (r, port)));
}

TEST_F (test_fixreal, fixreal_eqv_p)
{
    EXPECT_TRUE (eqv_p (r_fixreal_new_si (r, 2, 3),
                        r_fixreal_new_si (r, 4, 6)));
}

TEST_F (test_floreal, r_floreal_new)
{
    rsexp actual = r_floreal_new (r, 1.5);

    EXPECT_FALSE (r_failure_p (actual));
    EXPECT_TRUE (r_floreal_p (actual));
    EXPECT_TRUE (floreal_from_sexp (actual)->value == 1.5);

    allocator_disruptor md;
    actual = r_floreal_new (r, 1.5);
    EXPECT_TRUE (r_failure_p (actual));
}

TEST_F (test_floreal, write_floreal)
{
    rsexp port = r_open_output_string (r);

    r_port_write (r, port, r_floreal_new (r, 1.5));
    EXPECT_TRUE (equal_p (r_string_new (r, "1.500000"),
                          r_get_output_string (r, port)));
}

TEST_F (test_fixreal, floreal_eqv_p)
{
    EXPECT_FALSE (eqv_p (r_floreal_new (r, 0.),
                         r_floreal_new (r, 1.)));
}

TEST_F (test_complex, r_complex_new)
{
    {
        rsexp actual = r_complex_new (r, R_ZERO, r->flo_one);
        EXPECT_TRUE (r_failure_p (actual));
    }

    {
        rsexp actual = r_complex_new (r, r->flo_one, R_ZERO);
        EXPECT_TRUE (r_failure_p (actual));
    }

    {
        rsexp real = r->flo_one;
        rsexp imag = r->flo_zero;
        rsexp actual = r_complex_new (r, real, imag);

        EXPECT_TRUE (r_floreal_p (actual));
        EXPECT_TRUE (r_inexact_p (actual));
        EXPECT_TRUE (r_real_p (actual));
    }

    {
        rsexp real = r_fixreal_new_si (r, 2, 4);
        rsexp imag = r_fixreal_new_si (r, 0, 1);
        rsexp actual = r_complex_new (r, real, imag);

        EXPECT_TRUE (r_fixreal_p (actual));
        EXPECT_TRUE (r_exact_p (actual));
        EXPECT_TRUE (r_real_p (actual));
    }

    {
        rsexp real = r_fixreal_new_si (r, 2, 4);
        rsexp imag = r_fixreal_new_si (r, 1, 3);
        rsexp actual = r_complex_new (r, real, imag);

        EXPECT_FALSE (r_failure_p (actual));
        EXPECT_TRUE (r_exact_p (actual));

        allocator_disruptor md;
        actual = r_complex_new (r, real, imag);
        EXPECT_TRUE (r_failure_p (actual));
    }
}

TEST_F (test_complex, write_complex)
{
    rsexp port;

    {
        port = r_open_output_string (r);
        r_port_write (r, port, r_complex_new (r, R_ONE, R_ONE));
        EXPECT_TRUE (equal_p (r_string_new (r, "1+1i"),
                              r_get_output_string (r, port)));
    }

    {
        port = r_open_output_string (r);
        r_port_write (r, port, r_complex_new (r, R_ONE, R_ZERO));
        EXPECT_TRUE (equal_p (r_string_new (r, "1"),
                              r_get_output_string (r, port)));
    }
}

TEST_F (test_complex, complex_eqv_p)
{
    EXPECT_TRUE (eqv_p (r_complex_new (r, R_ONE, R_ONE),
                        r_complex_new (r, R_ONE, R_ONE)));
}

TEST_F (test_number, r_zero_p)
{
    EXPECT_TRUE (r_zero_p (R_ZERO));
    EXPECT_TRUE (r_zero_p (r->flo_zero));
    EXPECT_TRUE (r_zero_p (smi_to_fixreal (r, R_ZERO)));

    EXPECT_FALSE (r_zero_p (R_ONE));
    EXPECT_FALSE (r_zero_p (r_fixreal_new_si (r, 1, 1)));
    EXPECT_FALSE (r_zero_p (r->flo_one));
    EXPECT_FALSE (r_zero_p (r_complex_new (r, R_ZERO, R_ONE)));
}

TEST_F (test_number, r_sign)
{
    EXPECT_EQ (1, r_sign (R_ZERO));
    EXPECT_EQ (1, r_sign (r->flo_zero));
    EXPECT_EQ (1, r_sign (R_ONE));

    EXPECT_EQ (-1, r_sign (r_fixreal_new_si (r, -1, 1)));
    EXPECT_EQ (-1, r_sign (r_int_to_sexp (-1)));
}

TEST_F (test_number, r_real_p)
{
    EXPECT_TRUE (r_real_p (R_ZERO));
    EXPECT_TRUE (r_real_p (R_ONE));
}

TEST_F (test_number, r_real_part)
{
    EXPECT_EQ (R_ONE, r_real_part (r, R_ONE));
    EXPECT_EQ (R_ONE, r_real_part (r, r_complex_new (r, R_ONE, R_ONE)));
    EXPECT_TRUE (r_failure_p (r_real_part (r, R_FALSE)));
}

TEST_F (test_number, r_imag_part)
{
    rsexp n;

    n = r_floreal_new (r, 1.5);
    EXPECT_TRUE (eqv_p (r->flo_zero, r_imag_part (r, n)));
    EXPECT_EQ (R_ZERO, r_imag_part (r, R_ONE));

    n = r_complex_new (r, r->flo_one, r->flo_one);
    EXPECT_TRUE (eqv_p (r->flo_one, r_imag_part (r, n)));

    EXPECT_TRUE (r_failure_p (r_imag_part (r, R_FALSE)));
}
