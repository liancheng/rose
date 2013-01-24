#include "detail/math.h"
#include "detail/finer_number.h"
#include "rose/eq.h"
#include "rose/error.h"

#include <assert.h>

rsexp r_negate (RState* r, rsexp n)
{
    assert (r_small_int_p (n));
    return r_int_to_sexp (-r_int_from_sexp (n));
}

rsexp r_add (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));
    return r_int_to_sexp (r_int_from_sexp (lhs) + r_int_from_sexp (rhs));
}

rsexp r_minus (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));
    return r_int_to_sexp (r_int_from_sexp (lhs) - r_int_from_sexp (rhs));
}

rsexp r_multiply (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));
    return r_int_to_sexp (r_int_from_sexp (lhs) * r_int_from_sexp (rhs));
}

rsexp r_num_eq_p (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));
    return r_bool_to_sexp (r_int_from_sexp (lhs) == r_int_from_sexp (rhs));
}

rsexp r_num_lt_p (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));
    return r_bool_to_sexp (r_int_from_sexp (lhs) < r_int_from_sexp (rhs));
}

rsexp r_num_le_p (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));
    return r_bool_to_sexp (r_int_from_sexp (lhs) <= r_int_from_sexp (rhs));
}

rsexp r_num_gt_p (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));
    return r_bool_to_sexp (r_int_from_sexp (lhs) > r_int_from_sexp (rhs));
}

rsexp r_num_ge_p (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));
    return r_bool_to_sexp (r_int_from_sexp (lhs) >= r_int_from_sexp (rhs));
}
