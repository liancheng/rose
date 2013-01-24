#include "detail/number.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/math.h"

#include <assert.h>

static inline rsexp negate_smi (RState* r, rsexp n)
{
    return r_int_to_sexp (-r_int_from_sexp (n));
}

static inline rsexp negate_fixreal (RState* r, rsexp n)
{
    mpq_t neg;
    rsexp res;

    mpq_init (neg);
    mpq_set (neg, fixreal_value (n));
    mpq_neg (neg, neg);
    res = r_fixreal_new (r, neg);
    mpq_clear (neg);

    return res;
}

static inline rsexp negate_floreal (RState* r, rsexp n)
{
    return r_floreal_new (r, -floreal_value (n));
}

static inline rsexp negate_real (RState* r, rsexp n)
{
    if (r_small_int_p (n))
        return negate_smi (r, n);

    if (r_fixreal_p (n))
        return negate_fixreal (r, n);

    if (r_floreal_p (n))
        return negate_floreal (r, n);

    assert (0);
    return R_FAILURE;
}

static inline rsexp negate_complex (RState* r, rsexp n)
{
    rsexp neg_real;
    ensure (neg_real = r_negate (r, complex_real (n)));
    return r_complex_new (r, neg_real, complex_imag (n));
}

rsexp r_negate (RState* r, rsexp n)
{
    if (r_real_p (n))
        return negate_real (r, n);

    if (r_complex_p (n))
        return negate_complex (r, n);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, n);
    return R_FAILURE;
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
