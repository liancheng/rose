#include <gmpxx.h>

extern "C" {

#include "rose/eq.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/vector.h"

}

#include <gtest/gtest.h>

TEST (test_vector, r_vector_new)
{
    rsexp actual;

    actual = r_vector_new (1, R_TRUE);

    EXPECT_EQ (1u, r_vector_length (actual));
    EXPECT_PRED2 (r_eq_p, R_TRUE, r_vector_ref (actual, 0));
}

TEST (test_vector, vector)
{
    rsexp actual;

    actual = r_vector (2, r_int_to_sexp (1), r_int_to_sexp (2));

    EXPECT_EQ (2u, r_vector_length (actual));
    EXPECT_PRED2 (r_eq_p, r_int_to_sexp (1), r_vector_ref (actual, 0));
    EXPECT_PRED2 (r_eq_p, r_int_to_sexp (2), r_vector_ref (actual, 1));
}

TEST (test_vector, r_vector_to_list)
{
    rsexp actual;
    rsexp expected;

    actual = r_vector_to_list(r_vector (3,
                                        r_int_to_sexp (1),
                                        r_int_to_sexp (2),
                                        r_int_to_sexp (3)));

    expected = r_list (3,
                       r_int_to_sexp (1),
                       r_int_to_sexp (2),
                       r_int_to_sexp (3));

    EXPECT_PRED2 (r_equal_p, expected, actual);
}

TEST (test_vector, r_list_to_vector)
{
    rsexp actual;
    rsexp expected;

    actual = r_list_to_vector (r_list (3,
                                       r_int_to_sexp (1),
                                       r_int_to_sexp (2),
                                       r_int_to_sexp (3)));

    expected = r_vector (3,
                         r_int_to_sexp (1),
                         r_int_to_sexp (2),
                         r_int_to_sexp (3));

    EXPECT_PRED2 (r_equal_p, expected, actual);
}
