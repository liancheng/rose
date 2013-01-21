#include "detail/io.h"
#include "detail/math_workaround.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "detail/state.h"
#include "rose/string.h"

#include <assert.h>
#include <string.h>

static rsexp write_fixnum (RState* r, rsexp port, rsexp obj)
{
    RFixnum* fixnum = fixnum_from_sexp (obj);
    FILE* stream = port_to_stream (port);

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

    if (fixnum == NULL)
        return NULL;

    mpq_inits (fixnum->real, fixnum->imag, NULL);
    return fixnum;
}

rsexp smi_to_fixnum (RState* r, rsexp smi)
{
    mpq_t real;
    RFixnum* fixnum;
    rsexp res;

    mpq_init (real);
    mpq_set_si (real, r_int_from_sexp (smi), 1);

    fixnum = fixnum_new (r);

    if (!fixnum) {
        res = R_FAILURE;
        goto exit;
    }

    mpq_set (fixnum->real, real);
    res = fixnum_to_sexp (fixnum);

exit:
    mpq_clear (real);

    return res;
}

rbool r_fixnum_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_FIXNUM;
}

rbool r_flonum_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_FLONUM;
}

rsexp r_cstr_to_number (RState* r, rconstcstring text)
{
    RNumberReader reader;
    rsexp res;

    r_number_reader_init (r, &reader);
    res = r_number_read (&reader, text);

    return res;
}

rsexp r_string_to_number (RState* r, rsexp text)
{
    RNumberReader reader;
    rsexp res;

    r_number_reader_init (r, &reader);
    res = r_number_read (&reader, r_string_to_cstr (text));

    return res;
}

rsexp r_fixnum_new (RState* r, mpq_t real, mpq_t imag)
{
    RFixnum* fixnum;
    rsexp res;

    res = try_small_int (real, imag);

    if (!r_false_p (res))
        return res;

    fixnum = r_object_new (r, RFixnum, R_TAG_FIXNUM);

    if (fixnum == NULL)
        return R_FAILURE;

    mpq_inits (fixnum->real, fixnum->imag, NULL);
    mpq_set (fixnum->real, real);
    mpq_set (fixnum->imag, imag);

    return fixnum_to_sexp (fixnum);
}

rsexp r_fixint_new (RState* r, rint real)
{
    RFixnum* fixnum;
    rsexp smi;

    smi = r_int_to_sexp (real);

    if (r_int_from_sexp (smi) == real)
        return smi;

    fixnum = fixnum_new (r);
    if (!fixnum)
        return R_FAILURE;

    mpq_set_si (fixnum->real, real, 1);

    return fixnum_to_sexp (fixnum);
}

rsexp r_fixuint_new (RState* r, ruint real)
{
    RFixnum* fixnum;
    rsexp smi;

    smi = r_uint_to_sexp (real);

    if (r_uint_from_sexp (smi) == real)
        return smi;

    fixnum = fixnum_new (r);
    if (!fixnum)
        return R_FAILURE;

    mpq_set_ui (fixnum->real, real, 1);

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

rbool r_integer_p (rsexp obj)
{
    if (r_small_int_p (obj))
        return TRUE;

    if (r_fixnum_p (obj)) {
        mpq_t* real;
        mpq_t* imag;

        real = &fixnum_from_sexp (obj)->real;
        imag = &fixnum_from_sexp (obj)->imag;

        if (mpz_cmp_si (mpq_denref (*real), 1) != 0)
            return FALSE;

        if (mpz_cmp_si (mpq_numref (*imag), 0) != 0)
            return FALSE;

        return TRUE;
    }

    if (r_flonum_p (obj)) {
        double real = flonum_from_sexp (obj)->real;
        double imag = flonum_from_sexp (obj)->imag;

        return (imag == 0.) && r_ceil (real) == real;
    }

    return FALSE;
}

rbool r_real_p (rsexp obj)
{
    if (r_small_int_p (obj))
        return TRUE;

    if (r_fixnum_p (obj)) {
        mpq_t* imag = &fixnum_from_sexp (obj)->imag;
        return mpz_cmp_si (mpq_numref (*imag), 0) == 0;
    }

    if (r_flonum_p (obj)) {
        double imag = flonum_from_sexp (obj)->imag;
        return imag == 0.;
    }

    return FALSE;
}

rbool r_exact_p (rsexp obj)
{
    return r_small_int_p (obj) || r_fixnum_p (obj);
}

rsexp r_exact_to_inexact (RState* r, rsexp num)
{
    if (r_small_int_p (num))
        return r_flonum_new (r, r_cast (double, r_int_from_sexp (num)), 0.);

    if (r_flonum_p (num))
        return num;

    if (r_fixnum_p (num)) {
        double real = mpq_get_d (fixnum_from_sexp (num)->real);
        double imag = mpq_get_d (fixnum_from_sexp (num)->imag);
        return r_flonum_new (r, real, imag);
    }

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, num);

    return R_FAILURE;
}

rsexp r_inexact_to_exact (RState* r, rsexp num)
{
    mpq_t real;
    mpq_t imag;
    rsexp res;

    if (r_small_int_p (num) || r_fixnum_p (num))
        return num;

    if (!r_flonum_p (num)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, num);
        return R_FAILURE;
    }

    mpq_inits (real, imag, NULL);

    mpq_set_d (real, flonum_from_sexp (num)->real);
    mpq_set_d (imag, flonum_from_sexp (num)->imag);
    res = r_fixnum_new (r, real, imag);

    mpq_clears (real, imag, NULL);

    return res;
}

RTypeInfo fixnum_type = {
    .size = sizeof (RFixnum),
    .name = "fixnum",
    .ops = {
        .write = write_fixnum,
        .display = write_fixnum,
        .eqv_p = fixnum_eqv_p,
        .equal_p = fixnum_eqv_p,
        .finalize = fixnum_finalize
    }
};

RTypeInfo flonum_type = {
    .size = sizeof (RFlonum),
    .name = "flonum",
    .ops = {
        .write = write_flonum,
        .display = write_flonum,
        .eqv_p = flonum_eqv_p,
        .equal_p = flonum_eqv_p,
    }
};

/* ========================================================================== */

static rsexp write_fixreal (RState* r, rsexp port, rsexp obj)
{
    RFixreal* n = fixreal_from_sexp (obj);
    FILE* stream = port_to_stream (port);

    if (0 == mpq_out_str (stream, 10, n->value)) {
        r_error_code (r, R_ERR_UNKNOWN);
        return R_FAILURE;
    }

    return R_UNSPECIFIED;
}

static rbool fixreal_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    RFixreal* lhs_num = fixreal_from_sexp (lhs);
    RFixreal* rhs_num = fixreal_from_sexp (rhs);

    return mpq_cmp (lhs_num->value, rhs_num->value) == 0;
}

static void fixreal_finalize (RState* r, RObject* obj)
{
    RFixreal* n = fixreal_from_sexp (obj);
    mpq_clear (n->value);
}

rsexp r_fixreal_new (RState* r, mpq_t value)
{
    RFixreal* obj = r_object_new (r, RFixreal, R_TAG_FIXREAL);

    if (obj == NULL)
        return R_FAILURE;

    mpq_init (obj->value);
    mpq_set (obj->value, value);

    return fixreal_to_sexp (obj);
}

rsexp r_fixreal_new_si (RState* r, rint value)
{
    RFixreal* obj = r_object_new (r, RFixreal, R_TAG_FIXREAL);

    if (obj == NULL)
        return R_FAILURE;

    mpq_init (obj->value);
    mpq_set_si (obj->value, value, 1);

    return fixreal_to_sexp (obj);
}

RTypeInfo fixreal_type = {
    .size = sizeof (RFixreal),
    .name = "fixreal",
    .ops = {
        .write = write_fixreal,
        .display = write_fixreal,
        .eqv_p = fixreal_eqv_p,
        .equal_p = fixreal_eqv_p,
        .finalize = fixreal_finalize
    }
};
