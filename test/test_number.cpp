#include <gmpxx.h>

extern "C" {

#include "detail/sexp.h"
#include "rose/number.h"

}

#include <gtest/gtest.h>

class test_number_reader : public testing::Test {
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
