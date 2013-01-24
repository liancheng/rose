#include "utils.hpp"

#include "rose/eq.h"
#include "rose/finer_number.h"
#include "rose/pair.h"
#include "rose/string.h"
#include "rose/symbol.h"

class test_eq_p    : public fixture_base {};
class test_eqv_p   : public fixture_base {};
class test_equal_p : public fixture_base {};

TEST_F (test_eq_p, symbol)
{
    EXPECT_TRUE (eq_p (r_intern (r, "a"), r_intern (r, "a")));
}

TEST_F (test_eq_p, character)
{
    EXPECT_TRUE (eq_p (r_char_to_sexp ('a'), r_char_to_sexp ('a')));
}

TEST_F (test_eq_p, null)
{
    EXPECT_TRUE (eq_p (R_NULL, R_NULL));
}

TEST_F (test_eq_p, string)
{
    EXPECT_FALSE (eq_p (r_string_new (r, "a"), r_string_new (r, "a")));
}

TEST_F (test_eqv_p, small_int)
{
    EXPECT_TRUE (eqv_p (r_int_to_sexp (42), r_int_to_sexp (42)));
}

TEST_F (test_eqv_p, fixnum)
{
    EXPECT_TRUE (eqv_p (r_cstr_to_number (r, "1/2"),
                        r_cstr_to_number (r, "2/4")));
}

TEST_F (test_eqv_p, flonum)
{
    EXPECT_TRUE (eqv_p (r_cstr_to_number (r, "#i1/2"),
                        r_cstr_to_number (r, "0.5")));
}

TEST_F (test_equal_p, list)
{
    rsexp actual = r_list (r,
                           2,
                           r_string_new (r, "a"),
                           r_int_to_sexp (42));

    rsexp expected = r_cons (r,
                             r_string_new (r, "a"),
                             r_cons (r,
                                     r_int_to_sexp (42),
                                     R_NULL));

    EXPECT_TRUE (equal_p (expected, actual));
}
