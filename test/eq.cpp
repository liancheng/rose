#include <gmpxx.h>

extern "C" {

#include "rose/eq.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/string.h"
#include "rose/symbol.h"

}

#include <gtest/gtest.h>

TEST (test_eq_p, symbol)
{
    RState* state = r_state_new ();

    EXPECT_PRED2 (r_eq_p, r_symbol_new (state, "a"),
                          r_symbol_new (state, "a"));
}

TEST (test_eq_p, character)
{
    EXPECT_PRED2 (r_eq_p, r_char_to_sexp ('a'), r_char_to_sexp ('a'));
}

TEST (test_eq_p, null)
{
    EXPECT_PRED2 (r_eq_p, R_NULL, R_NULL);
}

TEST (test_eq_p, string)
{
    EXPECT_FALSE (r_eq_p (r_string_new ("a"), r_string_new ("a")));
}

TEST (test_eqv_p, small_int)
{
    EXPECT_PRED2 (r_eqv_p, r_int_to_sexp (42), r_int_to_sexp (42));
}

TEST (test_eqv_p, fixnum)
{
    EXPECT_PRED2 (r_eqv_p, r_string_to_number ("1/2"),
                           r_string_to_number ("2/4"));
}

TEST (test_eqv_p, flonum)
{
    EXPECT_PRED2 (r_eqv_p, r_string_to_number ("#i1/2"),
                           r_string_to_number ("0.5"));
}

TEST (test_equal_p, list)
{
    rsexp actual = r_list (2,
                           r_string_new ("a"),
                           r_int_to_sexp (42));

    rsexp expected = r_cons (r_string_new ("a"),
                             r_cons (r_int_to_sexp (42),
                                     R_NULL));

    EXPECT_PRED2 (r_equal_p, expected, actual);
}
