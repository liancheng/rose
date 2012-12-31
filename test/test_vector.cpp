#include "utils.hpp"

#include "rose/eq.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/vector.h"

class test_vector : public fixture_base {};

TEST_F (test_vector, r_vector_new)
{
    rsexp expected = r_uint_to_sexp (1u);
    rsexp actual   = r_vector_length (r_vector_new (r, 1, R_TRUE));

    EXPECT_EQ (expected, actual);
}

TEST_F (test_vector, r_vector_new_empty)
{
    rsexp v = r_vector_new (r, 0, R_TRUE);

    EXPECT_TRUE (r_vector_p (v));
    EXPECT_EQ(r_int_to_sexp (0), r_vector_length (v));
}

TEST_F (test_vector, r_vector)
{
    rsexp actual = r_vector (r, 2, r_int_to_sexp (1), r_int_to_sexp (2));

    EXPECT_EQ (r_uint_to_sexp (2u),
               r_vector_length (actual));

    EXPECT_TRUE (r_equal_p (r,
                            r_int_to_sexp (1),
                            r_vector_ref (r, actual, 0)));

    EXPECT_TRUE (r_equal_p (r,
                            r_int_to_sexp (2),
                            r_vector_ref (r, actual, 1)));
}

TEST_F (test_vector, r_vector_to_list)
{
    rsexp actual;
    rsexp expected;

    actual = r_vector_to_list(r,
                              r_vector (r,
                                        3,
                                        r_int_to_sexp (1),
                                        r_int_to_sexp (2),
                                        r_int_to_sexp (3)));

    expected = r_list (r,
                       3,
                       r_int_to_sexp (1),
                       r_int_to_sexp (2),
                       r_int_to_sexp (3));

    EXPECT_TRUE (r_equal_p (r, expected, actual));
}

TEST_F (test_vector, r_list_to_vector)
{
    rsexp actual;
    rsexp expected;

    actual = r_list_to_vector (r,
                               r_list (r,
                                       3,
                                       r_int_to_sexp (1),
                                       r_int_to_sexp (2),
                                       r_int_to_sexp (3)));

    expected = r_vector (r,
                         3,
                         r_int_to_sexp (1),
                         r_int_to_sexp (2),
                         r_int_to_sexp (3));

    EXPECT_TRUE (r_equal_p (r, expected, actual));
}
