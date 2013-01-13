#include "detail/env.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "detail/port.h"
#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/error.h"
#include "rose/gmp.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"

#include <assert.h>
#include <string.h>

static rsexp write_fixnum (RState* r, rsexp port, rsexp obj)
{
    RFixnum* fixnum = fixnum_from_sexp (obj);
    FILE*    stream = port_to_stream (port);

    if (0 == mpq_out_str (stream, 10, fixnum->real)) {
        r_error_code (r, R_ERR_UNKNOWN);
        return R_FAILURE;
    }

    if (0 != mpq_cmp_ui (fixnum->imag, 0u, 1u)) {
        if (0 < mpq_cmp_ui (fixnum->imag, 0u, 1u))
            ensure (r_port_write_char (r, port, '+'));

        mpq_out_str (stream, 10, fixnum->imag);
        ensure (r_port_write_char (r, port, 'i'));
    }

    return R_UNSPECIFIED;
}

static rsexp write_flonum (RState* r, rsexp port, rsexp obj)
{
    RFlonum* flonum = flonum_from_sexp (obj);

    ensure (r_port_printf (r, port, "%f", flonum->real));

    if (flonum->imag != 0.) {
        if (flonum->imag > 0.)
            ensure (r_port_write_char (r, port, '+'));

        ensure (r_port_printf (r, port, "%f", flonum->imag));
        ensure (r_port_write_char (r, port, 'i'));
    }

    return R_UNSPECIFIED;
}

static rsexp try_small_int (mpq_t real, mpq_t imag)
{
    rint smi;

    /* If imaginary part is not 0... */
    if (0 != mpq_cmp_ui (imag, 0u, 1u))
        return R_FALSE;

    mpq_canonicalize (real);

    /* If the denominator is not 1... */
    if (0 != mpz_cmp_ui (mpq_denref (real), 1u))
        return R_FALSE;

    /* If the number is too large (to fit into a signed int)... */
    if (!mpz_fits_sint_p (mpq_numref (real)))
        return R_FALSE;

    smi = mpz_get_si (mpq_numref (real));

    /* If the number doesn't fit into the range of small integers... */
    if (smi > R_SMI_MAX || smi < R_SMI_MIN)
        return R_FALSE;

    return r_int_to_sexp (smi);
}

static rbool fixnum_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    RFixnum* lhs_num = fixnum_from_sexp (lhs);
    RFixnum* rhs_num = fixnum_from_sexp (rhs);

    return mpq_cmp (lhs_num->real, rhs_num->real) == 0
        && mpq_cmp (lhs_num->imag, rhs_num->imag) == 0;
}

static rbool flonum_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    RFlonum* lhs_num = flonum_from_sexp (lhs);
    RFlonum* rhs_num = flonum_from_sexp (rhs);

    return lhs_num->real == rhs_num->real
        && lhs_num->imag == rhs_num->imag;
}

static void fixnum_finalize (RState* r, RObject* obj)
{
    RFixnum* fixnum = r_cast (RFixnum*, obj);
    mpq_clears (fixnum->real, fixnum->imag, NULL);
}

static RFixnum* fixnum_new (RState* r)
{
    RFixnum* fixnum = r_object_new (r, RFixnum, R_TAG_FIXNUM);
    mpq_inits (fixnum->real, fixnum->imag, NULL);
    return fixnum;
}

rbool r_fixnum_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_FIXNUM;
}

rbool r_flonum_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_FLONUM;
}

rsexp r_string_to_number (RState* r, rconstcstring text)
{
    RNumberReader reader;
    rsexp res;

    r_number_reader_init (r, &reader);
    res = r_number_read (&reader, text);

    return res;
}

rsexp r_fixnum_new (RState* r, mpq_t real, mpq_t imag)
{
    rsexp number = try_small_int (real, imag);

    if (!r_false_p (number))
        return number;

    RFixnum* fixnum = fixnum_new (r);
    mpq_set (fixnum->real, real);
    mpq_set (fixnum->imag, imag);

    return fixnum_to_sexp (fixnum);
}

rsexp r_fixreal_new (RState* r, mpq_t real)
{
    RFixnum* fixnum = fixnum_new (r);

    if (!fixnum)
        return R_FAILURE;

    mpq_set (fixnum->real, real);
    return fixnum_to_sexp (fixnum);
}

rsexp r_fixnum_normalize (rsexp obj)
{
    assert (r_fixnum_p (obj));

    rsexp smi = try_small_int (fixnum_from_sexp (obj)->real,
                               fixnum_from_sexp (obj)->imag);

    return r_false_p (smi) ? obj : smi;
}

rsexp r_flonum_new (RState* r, double real, double imag)
{
    RFlonum* flonum = r_object_new (r, RFlonum, R_TAG_FLONUM);

    if (!flonum)
        return R_FAILURE;

    flonum->real = real;
    flonum->imag = imag;

    return flonum_to_sexp (flonum);
}

rsexp r_int_to_sexp (rint n)
{
    assert (n >= R_SMI_MIN && n <= R_SMI_MAX);
    return (n << R_SMI_BITS) | R_TAG_SMI;
}

rint r_int_from_sexp (rsexp obj)
{
    assert (r_small_int_p (obj));
    return (r_cast (rint, obj)) >> R_SMI_BITS;
}

rbool r_byte_p (rsexp obj)
{
    if (!r_small_int_p (obj))
        return FALSE;

    rint i = r_int_from_sexp (obj);

    return i >= 0 && i <= 255;
}

rbool r_number_p (rsexp obj)
{
    return r_small_int_p (obj) || r_fixnum_p (obj) || r_flonum_p (obj);
}

rbool r_exact_p (rsexp obj)
{
    return r_small_int_p (obj) || r_fixnum_p (obj);
}

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

rsexp np_add (RState* r, rsexp args)
{
    return r_add (r, r_car (args), r_cadr (args));
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

void number_init_primitives (RState* r, rsexp* env)
{
    bind_primitive_x (r, env, "+", np_add, 2, 0, FALSE);
}

RTypeInfo fixnum_type = {
    .size = sizeof (RFixnum),
    .name = "fixnum",
    .ops = {
        .write    = write_fixnum,
        .display  = write_fixnum,
        .eqv_p    = fixnum_eqv_p,
        .equal_p  = fixnum_eqv_p,
        .finalize = fixnum_finalize
    }
};

RTypeInfo flonum_type = {
    .size = sizeof (RFlonum),
    .name = "flonum",
    .ops = {
        .write   = write_flonum,
        .display = write_flonum,
        .eqv_p   = flonum_eqv_p,
        .equal_p = flonum_eqv_p,
    }
};
