#include "utils.hpp"

#include "rose/eq.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/string.h"
#include "rose/symbol.h"

class test_eq_p    : public fixture_base {};
class test_eqv_p   : public fixture_base {};
class test_equal_p : public fixture_base {};

TEST_F (test_eq_p, symbol)
{
    EXPECT_TRUE (r_eq_p (state,
                         r_symbol_new (state, "a"),
                         r_symbol_new (state, "a")));
}

TEST_F (test_eq_p, character)
{
    EXPECT_TRUE (r_eq_p (state,
                         r_char_to_sexp ('a'),
                         r_char_to_sexp ('a')));
}

TEST_F (test_eq_p, null)
{
    EXPECT_TRUE (r_eq_p (state, R_NULL, R_NULL));
}

TEST_F (test_eq_p, string)
{
    EXPECT_FALSE (r_eq_p (state,
                          r_string_new (state, "a"),
                          r_string_new (state, "a")));
}

TEST_F (test_eqv_p, small_int)
{
    EXPECT_TRUE (r_eqv_p (state,
                          r_int_to_sexp (42),
                          r_int_to_sexp (42)));
}

TEST_F (test_eqv_p, fixnum)
{
    EXPECT_TRUE (r_eqv_p (state,
                          r_string_to_number (state, "1/2"),
                          r_string_to_number (state, "2/4")));
}

TEST_F (test_eqv_p, flonum)
{
    EXPECT_TRUE (r_eqv_p (state,
                          r_string_to_number (state, "#i1/2"),
                          r_string_to_number (state, "0.5")));
}

TEST_F (test_equal_p, list)
{
    rsexp actual = r_list (state,
                           2,
                           r_string_new (state, "a"),
                           r_int_to_sexp (42));

    rsexp expected = r_cons (state,
                             r_string_new (state, "a"),
                             r_cons (state,
                                     r_int_to_sexp (42),
                                     R_NULL));

    EXPECT_TRUE (r_equal_p (state, expected, actual));
}
