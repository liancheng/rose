#include "rose/eq.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/vector.h"

#include <glib.h>

static void test_vector_new ()
{
    rsexp actual;

    actual = r_vector_new (1, R_TRUE);

    g_assert (r_vector_length (actual) == 1u);
    g_assert (r_eq_p (r_vector_ref (actual, 0), R_TRUE));
}

static void test_vector ()
{
    rsexp actual;

    actual = r_vector (2, r_int_to_sexp (1), r_int_to_sexp (2));

    g_assert (r_vector_length (actual) == 2u);
    g_assert (r_eq_p (r_vector_ref (actual, 0), r_int_to_sexp (1)));
    g_assert (r_eq_p (r_vector_ref (actual, 1), r_int_to_sexp (2)));
}

static void test_vector_to_list ()
{
    rsexp actual;
    rsexp expected;
    rsexp input;

    input = r_vector (3,
                      r_int_to_sexp (1),
                      r_int_to_sexp (2),
                      r_int_to_sexp (3));

    expected = r_list (3,
                       r_int_to_sexp (1),
                       r_int_to_sexp (2),
                       r_int_to_sexp (3));

    actual = r_vector_to_list (input);

    g_assert (r_equal_p (expected, actual));
}

static void test_list_to_vector ()
{
    rsexp input;
    rsexp expected;

    input = r_list (3,
                    r_int_to_sexp (1),
                    r_int_to_sexp (2),
                    r_int_to_sexp (3));

    expected = r_vector (3,
                         r_int_to_sexp (1),
                         r_int_to_sexp (2),
                         r_int_to_sexp (3));

    g_assert (r_equal_p (expected, r_list_to_vector (input)));
}

void add_vector_test_suite ()
{
    g_test_add_func ("/vector/test_vector_new",
                     test_vector_new);

    g_test_add_func ("/vector/test_vector",
                     test_vector);

    g_test_add_func ("/vector/test_vector_to_list",
                     test_vector_to_list);

    g_test_add_func ("/vector/test_list_to_vector",
                     test_list_to_vector);
}
