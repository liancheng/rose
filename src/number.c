#include "detail/finer_number.h"
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
    FILE* stream = port_to_stream (port);

    if (0 == mpq_out_str (stream, 10, fixnum_real (obj))) {
        r_error_code (r, R_ERR_UNKNOWN);
        return R_FAILURE;
    }

    if (0 != mpq_cmp_ui (fixnum_imag (obj), 0u, 1u)) {
        if (0 < mpq_cmp_ui (fixnum_imag (obj), 0u, 1u))
            ensure (r_port_write_char (r, port, '+'));

        mpq_out_str (stream, 10, fixnum_imag (obj));
        ensure (r_port_write_char (r, port, 'i'));
    }

    return R_UNSPECIFIED;
}

static rsexp write_flonum (RState* r, rsexp port, rsexp obj)
{
    ensure (r_port_printf (r, port, "%f", flonum_real (obj)));

    if (flonum_imag (obj) != 0.) {
        if (flonum_imag (obj) > 0.)
            ensure (r_port_write_char (r, port, '+'));

        ensure (r_port_printf (r, port, "%f", flonum_imag (obj)));
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
    return r_fixreal_new_si (r, real, 1);
}

rsexp r_fixuint_new (RState* r, ruint real)
{
    return r_fixreal_new_ui (r, real, 1);
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
    return r_exact_p (obj) || r_inexact_p (obj);
}

rbool r_integer_p (rsexp obj)
{
    if (r_small_int_p (obj))
        return TRUE;

    if (r_fixnum_p (obj)) {
        if (mpz_cmp_si (mpq_denref (fixnum_real (obj)), 1) != 0)
            return FALSE;

        if (mpz_cmp_si (mpq_numref (fixnum_imag (obj)), 0) != 0)
            return FALSE;

        return TRUE;
    }

    if (r_flonum_p (obj)) {
        double real = flonum_real (obj);
        double imag = flonum_imag (obj);
        return (imag == 0.) && r_ceil (real) == real;
    }

    if (r_fixreal_p (obj))
        return mpz_cmp_si (mpq_denref (fixreal_value (obj)), 1) == 0;

    if (r_floreal_p (obj))
        return r_ceil (floreal_value (obj)) == floreal_value (obj);

    return FALSE;
}

rbool r_real_p (rsexp obj)
{
    if (r_small_int_p (obj))
        return TRUE;

    if (r_fixnum_p (obj))
        return mpz_cmp_si (mpq_numref (fixnum_imag (obj)), 0) == 0;

    if (r_flonum_p (obj)) {
        double imag = flonum_imag (obj);
        return imag == 0.;
    }

    if (r_fixreal_p (obj) || r_floreal_p (obj))
        return TRUE;

    return FALSE;
}

rbool r_exact_p (rsexp obj)
{
    return r_small_int_p (obj)
        || r_fixnum_p (obj)
        || r_fixreal_p (obj)
        || r_fixcomplex_p (obj);
}

rbool r_inexact_p (rsexp obj)
{
    return r_flonum_p (obj)
        || r_floreal_p (obj)
        || r_flocomplex_p (obj);
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
