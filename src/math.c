#include "detail/env.h"
#include "detail/number.h"
#include "rose/error.h"
#include "rose/math.h"
#include "rose/pair.h"

#include <assert.h>

static rsexp add_smi_smi (RState* r, rsexp lhs, rsexp rhs)
{
    int int_lhs, int_rhs, sum;
    mpq_t mpq_lhs, zero;

    int_lhs = r_int_from_sexp (lhs);
    int_rhs = r_int_from_sexp (rhs);
    sum = int_lhs + int_rhs;

    if ((int_lhs >= 0 && sum >= int_rhs && sum <= R_SMI_MAX) ||
            (int_lhs < 0 && sum <= int_rhs && sum >= R_SMI_MIN))
        return r_int_to_sexp (sum);

    mpq_inits (mpq_lhs, zero, NULL);
    mpq_set_si (mpq_lhs, int_lhs, 1u);

    if (int_rhs > 0)
        mpz_add_ui (mpq_numref (mpq_lhs),
                    mpq_numref (mpq_lhs),
                    r_cast (ruint, int_rhs));
    else
        mpz_sub_ui (mpq_numref (mpq_lhs),
                    mpq_numref (mpq_lhs),
                    r_cast (ruint, -int_rhs));

    return r_fixnum_new (r, mpq_lhs, zero);
}

static rsexp add_fix_smi (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* fixnum = fixnum_from_sexp (lhs);
    rint int_rhs = r_int_from_sexp (rhs);

    if (int_rhs > 0)
        mpz_add_ui (mpq_numref (fixnum->real),
                    mpq_numref (fixnum->real),
                    r_cast (ruint, int_rhs));
    else
        mpz_sub_ui (mpq_numref (fixnum->real),
                    mpq_numref (fixnum->real),
                    r_cast (ruint, -int_rhs));

    return fixnum_to_sexp (fixnum);
}

static rsexp add_flo_smi (RState* r, rsexp lhs, rsexp rhs)
{
    RFlonum* flonum;

    flonum = flonum_from_sexp (lhs);
    flonum->real += r_int_from_sexp (rhs);

    return flonum_to_sexp (flonum);
}

static rsexp add_smi_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return add_smi_smi (r, lhs, rhs);

    if (r_fixnum_p (rhs))
        return add_fix_smi (r, rhs, lhs);

    if (r_flonum_p (rhs))
        return add_flo_smi (r, rhs, lhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, rhs);

    return R_FAILURE;
}

static rsexp add_fix_fix (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* fix_lhs;
    RFixnum* fix_rhs;
    mpq_t real_sum;
    mpq_t imag_sum;

    fix_lhs = fixnum_from_sexp (lhs);
    fix_rhs = fixnum_from_sexp (rhs);

    mpq_inits (real_sum, imag_sum, NULL);
    mpq_add (real_sum, fix_lhs->real, fix_rhs->real);
    mpq_add (imag_sum, fix_lhs->imag, fix_rhs->imag);

    return r_fixnum_new (r, real_sum, imag_sum);
}

static rsexp add_fix_any (RState* r, rsexp lhs, rsexp rhs)
{
    if (r_small_int_p (rhs))
        return add_fix_smi (r, rhs, lhs);

    if (r_fixnum_p (rhs))
        return add_fix_fix (r, lhs, rhs);

    return R_FAILURE;
}

static rsexp np_add (RState* r, rsexp args)
{
    return r_add (r, r_car (args), r_cadr (args));
}

static rsexp np_minus (RState* r, rsexp args)
{
    return r_minus (r, r_car (args), r_cadr (args));
}

static rsexp np_multiply (RState* r, rsexp args)
{
    return r_multiply (r, r_car (args), r_cadr (args));
}

static rsexp np_divide (RState* r, rsexp args)
{
    return r_divide (r, r_car (args), r_cadr (args));
}

static rsexp np_modulo (RState* r, rsexp args)
{
    return r_modulo (r, r_car (args), r_cadr (args));
}

static rsexp np_equal (RState* r, rsexp args)
{
    return r_equal (r, r_car (args), r_cadr (args));
}

rsexp r_add (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_number_p (lhs));
    assert (r_number_p (rhs));

    if (r_small_int_p (lhs))
        return add_smi_any (r, lhs, rhs);

    if (r_fixnum_p (lhs))
        return add_fix_any (r, lhs, rhs);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, lhs);

    return R_FAILURE;
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

rsexp r_divide (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));

    return r_int_to_sexp (r_int_from_sexp (lhs) / r_int_from_sexp (rhs));
}

rsexp r_modulo (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));

    return r_int_to_sexp (r_int_from_sexp (lhs) % r_int_from_sexp (rhs));
}

rsexp r_equal (RState* r, rsexp lhs, rsexp rhs)
{
    assert (r_small_int_p (lhs));
    assert (r_small_int_p (rhs));

    return r_bool_to_sexp (lhs == rhs);
}

void math_init_primitives (RState* r, rsexp* env)
{
    bind_primitive_x (r, env, "+",      np_add,         2, 0, FALSE);
    bind_primitive_x (r, env, "-",      np_minus,       2, 0, FALSE);
    bind_primitive_x (r, env, "*",      np_multiply,    2, 0, FALSE);
    bind_primitive_x (r, env, "/",      np_divide,      2, 0, FALSE);
    bind_primitive_x (r, env, "=",      np_equal,       2, 0, FALSE);
    bind_primitive_x (r, env, "modulo", np_modulo,      2, 0, FALSE);
}

