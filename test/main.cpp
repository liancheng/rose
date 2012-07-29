#include <gmpxx.h>

extern "C" {

#include "rose/number.h"

}

#include <gc/gc.h>
#include <gtest/gtest.h>

TEST (smi_limit_test, number_test)
{
    ASSERT_TRUE (r_small_int_p (r_int_to_sexp (R_SMU_MAX)));
    ASSERT_TRUE (r_small_int_p (r_int_to_sexp (R_SMU_MIN)));
}

int main (int argc, char* argv [])
{
    GC_INIT ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
