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
    rsexp neg;

    ensure (neg = smi_to_fixreal (r, R_ZERO));
    mpq_neg (fixreal_value (neg), fixreal_value (n));

    return neg;
}

static inline rsexp negate_floreal (RState* r, rsexp n)
{
    return r_floreal_new (r, -floreal_value (n));
}

static inline rsexp negate_real (RState* r, rsexp n)
{
    if (r_small_int_p (n))
        return negate_smi (r, n);

    assert (r_fixreal_p (n));
    return negate_fixreal (r, n);
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

static inline rsexp add_flo_smi (RState* r, rsexp lhs, rsexp rhs)
{
    return r_floreal_new (r, floreal_value (lhs) + r_int_from_sexp (rhs));
}

static inline rsexp add_flo_fix (RState* r, rsexp lhs, rsexp rhs)
{
    double sum = floreal_value (lhs) + mpq_get_d (fixreal_value (rhs));
    return r_floreal_new (r, sum);
}

static inline rsexp add_flo_flo (RState* r, rsexp lhs, rsexp rhs)
{
    return r_floreal_new (r, floreal_value (lhs) + floreal_value (rhs));
}

static inline rsexp add_flo_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return add_flo_smi (r, lhs, rhs);

    if (r_fixreal_p (rhs))
        return add_flo_fix (r, lhs, rhs);

    assert (r_floreal_p (rhs));
    return add_flo_flo (r, lhs, rhs);
}

static inline rsexp add_fix_fix (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp sum;

    ensure (sum = smi_to_fixreal (r, R_ZERO));

    mpq_add (fixreal_value (sum),
             fixreal_value (lhs),
             fixreal_value (rhs));

    return fixreal_normalize (sum);
}

static inline rsexp add_fix_smi (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp fix_rhs;
    ensure (fix_rhs = smi_to_fixreal (r, rhs));
    return add_fix_fix (r, lhs, fix_rhs);
}

static inline rsexp add_fix_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return add_fix_smi (r, lhs, rhs);

    if (r_fixreal_p (rhs))
        return add_fix_fix (r, lhs, rhs);

    assert (r_floreal_p (rhs));
    return add_flo_fix (r, rhs, lhs);
}

static inline rbool smi_sum_overflow_p (rintw lhs, rintw rhs)
{
    rintw sum = lhs + rhs;
    return r_int_from_sexp (r_int_to_sexp (sum)) != sum;
}

static inline rsexp add_smi_smi (RState* r, rsexp lhs, rsexp rhs)
{
    rintw smi_lhs, smi_rhs;
    rsexp fix_lhs, fix_rhs;

    smi_lhs = r_int_from_sexp (lhs);
    smi_rhs = r_int_from_sexp (rhs);

    if (!smi_sum_overflow_p (smi_lhs, smi_rhs))
        return r_int_to_sexp (smi_lhs + smi_rhs);

    ensure (fix_lhs = smi_to_fixreal (r, lhs));
    ensure (fix_rhs = smi_to_fixreal (r, rhs));

    return add_fix_fix (r, fix_lhs, fix_rhs);
}

static inline rsexp add_smi_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return add_smi_smi (r, lhs, rhs);

    if (r_fixreal_p (rhs))
        return add_fix_smi (r, rhs, lhs);

    assert (r_floreal_p (rhs));
    return add_flo_smi (r, rhs, lhs);
}

static inline rsexp add_real (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (lhs))
        return add_smi_any (r, lhs, rhs);

    if (r_fixreal_p (lhs))
        return add_fix_any (r, lhs, rhs);

    assert (r_floreal_p (lhs));
    return add_flo_any (r, lhs, rhs);
}

rsexp r_add (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp lhs_real, rhs_real, real;
    rsexp lhs_imag, rhs_imag, imag;

    if (r_zero_p (lhs) && r_number_p (rhs))
        return rhs;

    if (r_zero_p (rhs) && r_number_p (lhs))
        return lhs;

    ensure (lhs_real = r_real_part (r, lhs));
    ensure (rhs_real = r_real_part (r, rhs));
    ensure (lhs_imag = r_imag_part (r, lhs));
    ensure (rhs_imag = r_imag_part (r, rhs));

    ensure (real = add_real (r, lhs_real, rhs_real));
    ensure (imag = add_real (r, lhs_imag, rhs_imag));

    return r_complex_new (r, real, imag);
}

rsexp r_minus (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp rhs_neg;
    ensure (rhs_neg = r_negate (r, rhs));
    return r_add (r, lhs, rhs_neg);
}

static inline rsexp multiply_flo_smi (RState* r, rsexp lhs, rsexp rhs)
{
    return r_floreal_new (r, floreal_value (lhs) * r_int_from_sexp (rhs));
}

static inline rsexp multiply_flo_fix (RState* r, rsexp lhs, rsexp rhs)
{
    double prod = floreal_value (lhs) * mpq_get_d (fixreal_value (rhs));
    return r_floreal_new (r, prod);
}

static inline rsexp multiply_flo_flo (RState* r, rsexp lhs, rsexp rhs)
{
    return r_floreal_new (r, floreal_value (lhs) * floreal_value (rhs));
}

static inline rsexp multiply_flo_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return multiply_flo_smi (r, lhs, rhs);

    if (r_fixreal_p (rhs))
        return multiply_flo_fix (r, lhs, rhs);

    assert (r_floreal_p (rhs));
    return multiply_flo_flo (r, lhs, rhs);
}

static inline rsexp multiply_fix_fix (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp prod;

    ensure (prod = smi_to_fixreal (r, R_ZERO));

    mpq_mul (fixreal_value (prod),
             fixreal_value (lhs),
             fixreal_value (rhs));

    return fixreal_normalize (prod);
}

static inline rsexp multiply_fix_smi (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp fix_rhs;
    ensure (fix_rhs = smi_to_fixreal (r, rhs));
    return multiply_fix_fix (r, lhs, fix_rhs);
}

static inline rsexp multiply_fix_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return multiply_fix_smi (r, lhs, rhs);

    if (r_fixreal_p (rhs))
        return multiply_fix_fix (r, lhs, rhs);

    assert (r_floreal_p (rhs));
    return multiply_flo_fix (r, rhs, lhs);
}

static inline rbool smi_product_overflow_p (rintw lhs, rintw rhs)
{
    rintw prod = lhs * rhs;

    return prod / lhs != rhs
        || r_int_from_sexp (r_int_to_sexp (prod)) != prod;
}

static inline rsexp multiply_smi_smi (RState* r, rsexp lhs, rsexp rhs)
{
    rintw smi_lhs, smi_rhs, prod;
    rsexp fix_lhs, fix_rhs;

    smi_lhs = r_int_from_sexp (lhs);
    smi_rhs = r_int_from_sexp (rhs);
    prod = smi_lhs * smi_rhs;

    if (prod == 0 || !smi_product_overflow_p (smi_lhs, smi_rhs))
        return r_int_to_sexp (prod);

    ensure (fix_lhs = smi_to_fixreal (r, lhs));
    ensure (fix_rhs = smi_to_fixreal (r, rhs));

    return multiply_fix_fix (r, fix_lhs, fix_rhs);
}

static inline rsexp multiply_smi_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return multiply_smi_smi (r, lhs, rhs);

    if (r_fixreal_p (rhs))
        return multiply_fix_smi (r, rhs, lhs);

    assert (r_floreal_p (rhs));
    return multiply_flo_smi (r, rhs, lhs);
}

static inline rsexp multiply_real (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (lhs))
        return multiply_smi_any (r, lhs, rhs);

    if (r_fixreal_p (lhs))
        return multiply_fix_any (r, lhs, rhs);

    assert (r_floreal_p (lhs));
    return multiply_flo_any (r, rhs, lhs);
}

rsexp r_multiply (RState* r, rsexp lhs, rsexp rhs)
{
    /* lhs = a + bi, rhs = c + di */

    rsexp a, b, c, d;
    rsexp ac, bd, ad, bc;
    rsexp real, imag;

    if (r_zero_p (lhs) && r_number_p (rhs))
        return lhs;

    if (r_zero_p (rhs) && r_number_p (lhs))
        return rhs;

    if (r_one_p (lhs) && r_number_p (rhs))
        return rhs;

    if (r_one_p (rhs) && r_number_p (lhs))
        return lhs;

    ensure (a = r_real_part (r, lhs));
    ensure (b = r_imag_part (r, lhs));
    ensure (c = r_real_part (r, rhs));
    ensure (d = r_imag_part (r, rhs));

    ensure (ac = multiply_real (r, a, c));
    ensure (bd = multiply_real (r, b, d));
    ensure (ad = multiply_real (r, a, d));
    ensure (bc = multiply_real (r, b, c));

    ensure (real = r_minus (r, ac, bd));
    ensure (imag = add_real (r, ad, bc));

    return r_complex_new (r, real, imag);
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
