#include "rose/eq.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/vector.h"

#include <gtest/gtest.h>

class test_vector : public testing::Test {
protected:
    RState* state;

    virtual void SetUp ()
    {
        state = r_state_open ();
    }

    virtual void TearDown ()
    {
        r_state_free (state);
    }

};

TEST_F (test_vector, r_vector_new)
{
    rsexp actual;

    actual = r_vector_new (state, 1, R_TRUE);

    EXPECT_EQ (1u, r_vector_length (actual));
    EXPECT_TRUE (r_equal_p (state, R_TRUE, r_vector_ref (actual, 0)));
}

TEST_F (test_vector, r_vector)
{
    rsexp actual;

    actual = r_vector (state, 2, r_int_to_sexp (1), r_int_to_sexp (2));

    EXPECT_EQ (2u, r_vector_length (actual));

    EXPECT_TRUE (r_equal_p (state,
                            r_int_to_sexp (1),
                            r_vector_ref (actual, 0)));

    EXPECT_TRUE (r_equal_p (state,
                            r_int_to_sexp (2),
                            r_vector_ref (actual, 1)));
}

TEST_F (test_vector, r_vector_to_list)
{
    rsexp actual;
    rsexp expected;

    actual = r_vector_to_list(state,
                              r_vector (state,
                                        3,
                                        r_int_to_sexp (1),
                                        r_int_to_sexp (2),
                                        r_int_to_sexp (3)));

    expected = r_list (state,
                       3,
                       r_int_to_sexp (1),
                       r_int_to_sexp (2),
                       r_int_to_sexp (3));

    EXPECT_TRUE (r_equal_p (state, expected, actual));
}

TEST_F (test_vector, r_list_to_vector)
{
    rsexp actual;
    rsexp expected;

    actual = r_list_to_vector (state,
                               r_list (state,
                                       3,
                                       r_int_to_sexp (1),
                                       r_int_to_sexp (2),
                                       r_int_to_sexp (3)));

    expected = r_vector (state,
                         3,
                         r_int_to_sexp (1),
                         r_int_to_sexp (2),
                         r_int_to_sexp (3));

    EXPECT_TRUE (r_equal_p (state, expected, actual));
}
