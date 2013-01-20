#include "detail/math.h"
#include "detail/number.h"
#include "rose/eq.h"
#include "rose/error.h"

#include <assert.h>

rsexp negate_smi (RState* r, rsexp num)
{
    return r_int_to_sexp (-r_int_from_sexp (num));
}

rsexp negate_flo (RState* r, rsexp num)
{
    RFlonum* flonum = flonum_from_sexp (num);
    return r_flonum_new (r, -flonum->real, flonum->imag);
}

rsexp negate_fix (RState* r, rsexp num)
{
    RFixnum* fixnum;
    mpq_t real;
    rsexp res;

    fixnum = fixnum_from_sexp (num);

    mpq_init (real);
    mpq_set (real, fixnum->real);
    mpq_neg (real, real);
    res = r_fixnum_new (r, real, fixnum->imag);
    mpq_clear (real);

    return res;
}

rsexp r_negate (RState* r, rsexp num)
{
    if (r_small_int_p (num))
        return negate_smi (r, num);

    if (r_flonum_p (num))
        return negate_flo (r, num);

    if (r_fixnum_p (num))
        return negate_fix (r, num);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, num);

    return R_FAILURE;
}

rbool smi_sum_overflow_p (int lhs, int rhs, int sum)
{
    if ((lhs > 0 && sum < lhs) || (lhs < 0 && sum > rhs))
        return TRUE;

    return sum != r_int_from_sexp (r_int_to_sexp (sum));
}

rsexp add_smi_smi (RState* r, rsexp lhs, rsexp rhs)
{
    rint smi_lhs;
    rint smi_rhs;
    rint smi_sum;
    rsexp tmp;

    smi_lhs = r_int_from_sexp (lhs);
    smi_rhs = r_int_from_sexp (rhs);
    smi_sum = smi_lhs + smi_rhs;

    if (!smi_sum_overflow_p (smi_lhs, smi_rhs, smi_sum))
        return r_int_to_sexp (smi_sum);
    else {
        ensure (tmp = smi_to_fixnum (r, lhs));
        return add_fix_smi (r, tmp, rhs);
    }
}

rsexp add_smi_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (lhs == R_ZERO && r_number_p (rhs))
        return rhs;

    if (r_small_int_p (rhs))
        return add_smi_smi (r, lhs, rhs);

    if (r_fixnum_p (rhs))
        return add_fix_smi (r, rhs, lhs);

    if (r_flonum_p (rhs))
        return add_flo_smi (r, rhs, lhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

    return R_FAILURE;
}

rsexp add_flo_smi (RState* r, rsexp lhs, rsexp rhs)
{
    RFlonum* flo_lhs = flonum_from_sexp (lhs);

    return r_flonum_new (r,
                         flo_lhs->real + r_int_from_sexp (rhs),
                         flo_lhs->imag);
}

rsexp add_flo_flo (RState* r, rsexp lhs, rsexp rhs)
{
    RFlonum* flo_lhs = flonum_from_sexp (lhs);
    RFlonum* flo_rhs = flonum_from_sexp (rhs);

    return r_flonum_new (r,
                         flo_lhs->real + flo_rhs->real,
                         flo_lhs->imag + flo_rhs->imag);
}

rsexp add_flo_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return add_flo_smi (r, lhs, rhs);

    if (r_flonum_p (rhs))
        return add_flo_flo (r, lhs, rhs);

    if (r_fixnum_p (rhs))
        return add_fix_flo (r, rhs, lhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

    return R_FAILURE;
}

rsexp add_fix_smi (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* fix_lhs;
    mpq_t real_sum;
    rint smi_rhs;
    rsexp res;

    smi_rhs = r_int_from_sexp (rhs);
    if (smi_rhs == R_ZERO)
        return lhs;

    fix_lhs = fixnum_from_sexp (lhs);
    mpq_init (real_sum);

    if (smi_rhs > 0)
        mpz_add_ui (mpq_numref (real_sum),
                    mpq_numref (fix_lhs->real),
                    r_cast (ruint, smi_rhs));
    else
        mpz_sub_ui (mpq_numref (real_sum),
                    mpq_numref (fix_lhs->real),
                    r_cast (ruint, -smi_rhs));

    res = r_fixnum_new (r, real_sum, fix_lhs->imag);
    mpq_clear (real_sum);

    return res;
}

rsexp add_fix_fix (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* fix_lhs;
    RFixnum* fix_rhs;
    mpq_t real_sum;
    mpq_t imag_sum;
    rsexp res;

    fix_lhs = fixnum_from_sexp (lhs);
    fix_rhs = fixnum_from_sexp (rhs);

    mpq_inits (real_sum, imag_sum, NULL);
    mpq_add (real_sum, fix_lhs->real, fix_rhs->real);
    mpq_add (imag_sum, fix_lhs->imag, fix_rhs->imag);

    res = r_fixnum_new (r, real_sum, imag_sum);
    mpq_clears (real_sum, imag_sum, NULL);

    return res;
}

rsexp add_fix_flo (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* fix_lhs = fixnum_from_sexp (lhs);
    RFlonum* flo_rhs = flonum_from_sexp (rhs);
    double lhs_real = mpq_get_d (fix_lhs->real);
    double lhs_imag = mpq_get_d (fix_lhs->imag);

    return r_flonum_new (r,
                         lhs_real + flo_rhs->real,
                         lhs_imag + flo_rhs->imag);
}

rsexp add_fix_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return add_fix_smi (r, lhs, rhs);

    if (r_fixnum_p (rhs))
        return add_fix_fix (r, lhs, rhs);

    if (r_flonum_p (rhs))
        return add_fix_flo (r, lhs, rhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

    return R_FAILURE;
}

rsexp r_add (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (lhs))
        return add_smi_any (r, lhs, rhs);

    if (r_fixnum_p (lhs))
        return add_fix_any (r, lhs, rhs);

    if (r_flonum_p (lhs))
        return add_flo_any (r, lhs, rhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, lhs);

    return R_FAILURE;
}

rsexp r_minus (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp neg_rhs;
    ensure (neg_rhs = r_negate (r, rhs));
    return r_add (r, lhs, neg_rhs);
}

rbool smi_product_overflow_p (int lhs, int rhs, int prod)
{
    return (prod / lhs != rhs)
        || (prod != r_int_from_sexp (r_int_to_sexp (prod)));
}

/* lhs is neither 0 nor 1 */
rsexp multiply_smi_smi (RState* r, rsexp lhs, rsexp rhs)
{
    int smi_lhs;
    int smi_rhs;
    int smi_prod;
    rsexp tmp;

    if (rhs == R_ZERO)
        return R_ZERO;

    if (rhs == R_ONE)
        return lhs;

    smi_lhs = r_int_from_sexp (lhs);
    smi_rhs = r_int_from_sexp (rhs);
    smi_prod = smi_lhs * smi_rhs;

    if (!smi_product_overflow_p (smi_lhs, smi_rhs, smi_prod))
        return r_int_to_sexp (smi_prod);
    else {
        ensure (tmp = smi_to_fixnum (r, smi_lhs));
        return multiply_fix_smi (r, tmp, rhs);
    }
}

rsexp multiply_smi_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (lhs == R_ZERO)
        return lhs;

    if (lhs == R_ONE && r_number_p (rhs))
        return rhs;

    if (r_small_int_p (rhs))
        return multiply_smi_smi (r, lhs, rhs);

    if (r_fixnum_p (rhs))
        return multiply_fix_smi (r, rhs, lhs);

    if (r_flonum_p (rhs))
        return multiply_flo_smi (r, rhs, lhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

    return R_FAILURE;
}

rsexp multiply_flo_smi (RState* r, rsexp lhs, rsexp rhs)
{
    RFlonum* flo_lhs = flonum_from_sexp (lhs);
    int smi_rhs = r_int_from_sexp (rhs);

    return r_flonum_new (r,
                         flo_lhs->real * smi_rhs,
                         flo_lhs->imag * smi_rhs);
}

rsexp multiply_flo_flo (RState* r, rsexp lhs, rsexp rhs)
{
    RFlonum* flo_lhs = flonum_from_sexp (lhs);
    RFlonum* flo_rhs = flonum_from_sexp (rhs);
    double real = flo_lhs->real * flo_rhs->real +
                  flo_lhs->imag * flo_rhs->imag;
    double imag = flo_lhs->real * flo_rhs->imag +
                  flo_lhs->imag * flo_rhs->real;

    return r_flonum_new (r, real, imag);
}

rsexp multiply_flo_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs)) {
        if (rhs == R_ZERO)
            return R_ZERO;

        if (rhs == R_ONE)
            return lhs;

        return multiply_flo_smi (r, lhs, rhs);
    }

    if (r_fixnum_p (rhs))
        return multiply_fix_flo (r, rhs, lhs);

    if (r_flonum_p (rhs))
        return multiply_flo_flo (r, lhs, rhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

    return R_FAILURE;
}

/* lhs is neither 0 nor 1 */
rsexp multiply_fix_smi (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* fix_lhs;
    mpq_t prod_real;
    mpq_t prod_imag;
    mpq_t mpq_rhs;
    rsexp res;

    if (rhs == R_ZERO)
        return R_ZERO;

    if (rhs == R_ONE)
        return lhs;

    fix_lhs = fixnum_from_sexp (lhs);

    mpq_inits (prod_real, prod_imag, mpq_rhs, NULL);
    mpq_set_si (mpq_rhs, r_int_from_sexp (rhs), 1);

    mpq_mul (prod_real, fix_lhs->real, mpq_rhs);
    mpq_mul (prod_imag, fix_lhs->imag, mpq_rhs);

    res = r_fixnum_new (r, prod_real, prod_imag);
    mpq_clears (prod_real, prod_imag, mpq_rhs, NULL);

    return res;
}

/* Neither lhs nor rhs is 0 or 1 */
rsexp multiply_fix_fix (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* fix_lhs;
    RFixnum* fix_rhs;
    mpq_t prod_real;
    mpq_t prod_imag;
    mpq_t tmp1, tmp2;
    rsexp res;

    fix_lhs = fixnum_from_sexp (lhs);
    fix_rhs = fixnum_from_sexp (rhs);

    mpq_inits (prod_real, prod_imag, tmp1, tmp2, NULL);

    mpq_mul (tmp1, fix_lhs->real, fix_rhs->real);
    mpq_mul (tmp2, fix_lhs->imag, fix_rhs->imag);
    mpq_sub (prod_real, tmp1, tmp2);

    mpq_mul (tmp1, fix_lhs->real, fix_rhs->imag);
    mpq_mul (tmp2, fix_lhs->imag, fix_rhs->real);
    mpq_add (prod_real, tmp1, tmp2);

    res = r_fixnum_new (r, prod_real, prod_imag);
    mpq_clears (prod_real, prod_imag, tmp1, tmp2, NULL);

    return res;
}

rsexp multiply_fix_flo (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* fix_lhs = fixnum_from_sexp (lhs);
    RFlonum* flo_rhs = flonum_from_sexp (rhs);

    double lhs_real = mpq_get_d (fix_lhs->real);
    double lhs_imag = mpq_get_d (fix_lhs->imag);

    double prod_real = lhs_real * flo_rhs->real + lhs_imag * flo_rhs->imag;
    double prod_imag = lhs_real * flo_rhs->imag + lhs_imag * flo_rhs->real;

    return r_flonum_new (r, prod_real, prod_imag);
}

rsexp multiply_fix_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs)) {
        if (rhs == R_ZERO)
            return R_ZERO;

        if (rhs == R_ONE)
            return lhs;

        return multiply_fix_smi (r, lhs, rhs);
    }

    if (r_fixnum_p (rhs))
        return multiply_fix_fix (r, lhs, rhs);

    if (r_flonum_p (rhs))
        return multiply_fix_flo (r, lhs, rhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

    return R_FAILURE;
}

rsexp r_multiply (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (lhs))
        return multiply_smi_any (r, lhs, rhs);

    if (r_fixnum_p (lhs))
        return multiply_fix_any (r, lhs, rhs);

    if (r_flonum_p (lhs))
        return multiply_flo_any (r, lhs, rhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, lhs);

    return R_FAILURE;
}

rsexp invert_smi (RState* r, rsexp num)
{
    mpq_t real;

    mpq_init (real);
    mpq_set_si (real, 1, r_int_from_sexp (num));

    return r_fixreal_new (r, real);
}

rsexp invert_fix (RState* r, rsexp num)
{
    RFixnum* fixnum;
    mpq_t tmp1;
    mpq_t tmp2;
    mpq_t tmp3;
    rsexp res;

    fixnum = fixnum_from_sexp (num);
    mpq_inits (tmp1, tmp2, tmp3, NULL);

    if (mpq_cmp_ui (fixnum->imag, 0, 1) == 0) {
        mpq_inv (tmp1, fixnum->real);
        goto result;
    }

    mpq_mul (tmp1, fixnum->real, fixnum->real);
    mpq_mul (tmp2, fixnum->imag, fixnum->imag);
    mpq_add (tmp3, tmp1, tmp2);

    mpq_div (tmp1, fixnum->real, tmp3);
    mpq_div (tmp2, fixnum->imag, tmp3);
    mpq_neg (tmp2, tmp2);

result:
    res = r_fixnum_new (r, tmp1, tmp2);
    mpq_clears (tmp1, tmp2, tmp3, NULL);

    return res;
}

rsexp invert_flo (RState* r, rsexp num)
{
    RFlonum* flonum;
    double real;
    double imag;
    double tmp;

    flonum = flonum_from_sexp (num);

    if (flonum->imag == 0.0)
        return r_flonum_new (r, 1 / flonum->real, 0.);

    tmp = flonum->real * flonum->real + flonum->imag * flonum->imag;
    real = flonum->real / tmp;
    imag = -flonum->imag / tmp;

    return r_flonum_new (r, real, imag);
}

rsexp r_invert (RState* r, rsexp num)
{
    if (r_small_int_p (num))
        return invert_smi (r, num);

    if (r_fixnum_p (num))
        return invert_fix (r, num);

    if (r_flonum_p (num))
        return invert_flo (r, num);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, num);

    return R_FAILURE;
}

rsexp r_divide (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp tmp;

    ensure (tmp = r_invert (r, rhs));
    return r_multiply (r, lhs, tmp);
}

rsexp r_num_eq_p (RState* r, rsexp lhs, rsexp rhs)
{
    rsexp inexact;

    if (r_small_int_p (lhs)) {
        if (r_flonum_p (rhs)) {
            RFlonum* flonum = flonum_from_sexp (rhs);

            if (flonum->imag)
                return R_FALSE;

            return r_cast (double, r_int_from_sexp (lhs)) == flonum->real;
        }

        if (r_small_int_p (rhs))
            return r_bool_to_sexp (lhs == rhs);

        r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

        return R_FAILURE;
    }

    if (r_fixnum_p (lhs)) {
        if (r_small_int_p (rhs))
            return R_FALSE;

        if (r_fixnum_p (rhs))
            return r_bool_to_sexp (r_eqv_p (r, lhs, rhs));

        if (r_flonum_p (rhs)) {
            ensure (inexact = r_exact_to_inexact (r, lhs));
            return r_bool_to_sexp (r_eqv_p (r, inexact, rhs));
        }

        r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

        return R_FAILURE;
    }

    if (r_flonum_p (lhs)) {
        ensure (inexact = r_exact_to_inexact (r, rhs));
        return r_bool_to_sexp (r_eqv_p (r, lhs, inexact));
    }

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, lhs);

    return R_FAILURE;
}

rsexp r_num_cmp_helper (RState* r, rsexp lhs, rsexp rhs,
                        double* lhs_real, double* rhs_real)
{
    rsexp flo_lhs;
    rsexp flo_rhs;

    ensure (flo_lhs = r_exact_to_inexact (r, lhs));
    ensure (flo_rhs = r_exact_to_inexact (r, rhs));

    if (!r_real_p (flo_lhs)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, lhs);
        return R_FAILURE;
    }

    if (!r_real_p (flo_rhs)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);
        return R_FAILURE;
    }

    *lhs_real = flonum_from_sexp (flo_lhs)->real;
    *rhs_real = flonum_from_sexp (flo_rhs)->real;

    return R_UNSPECIFIED;
}

rsexp r_num_lt_p (RState* r, rsexp lhs, rsexp rhs)
{
    double lhs_real;
    double rhs_real;

    ensure (r_num_cmp_helper (r, lhs, rhs, &lhs_real, &rhs_real));
    return r_bool_to_sexp (lhs_real < rhs_real);
}

rsexp r_num_le_p (RState* r, rsexp lhs, rsexp rhs)
{
    double lhs_real;
    double rhs_real;

    ensure (r_num_cmp_helper (r, lhs, rhs, &lhs_real, &rhs_real));
    return r_bool_to_sexp (lhs_real <= rhs_real);
}

rsexp r_num_gt_p (RState* r, rsexp lhs, rsexp rhs)
{
    double lhs_real;
    double rhs_real;

    ensure (r_num_cmp_helper (r, lhs, rhs, &lhs_real, &rhs_real));
    return r_bool_to_sexp (lhs_real > rhs_real);
}

rsexp r_num_ge_p (RState* r, rsexp lhs, rsexp rhs)
{
    double lhs_real;
    double rhs_real;

    ensure (r_num_cmp_helper (r, lhs, rhs, &lhs_real, &rhs_real));
    return r_bool_to_sexp (lhs_real >= rhs_real);
}
