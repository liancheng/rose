#include "rose/number.h"

#include <glib.h>

static void test_smi_limit ()
{
    g_assert (r_small_int_p (r_int_to_sexp (R_SMI_MAX)));
    g_assert (r_small_int_p (r_int_to_sexp (R_SMI_MIN)));
}

void add_number_test_suite ()
{
    g_test_add_func ("/number/small_integer/test_limit", test_smi_limit);
}
