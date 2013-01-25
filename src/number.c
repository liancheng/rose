#include "detail/gc.h"
#include "detail/io.h"
#include "detail/math_workaround.h"
#include "detail/number.h"
#include "detail/number_reader.h"
#include "detail/state.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/gmp.h"
#include "rose/string.h"

#include <assert.h>

static rsexp fixreal_write (RState* r, rsexp port, rsexp obj)
{
    RFixreal* n = fixreal_from_sexp (obj);
    FILE* stream = port_to_stream (port);

    if (0 == mpq_out_str (stream, 10, n->value)) {
        r_error_code (r, R_ERR_UNKNOWN);
        return R_FAILURE;
    }

    return R_UNSPECIFIED;
}

static rsexp floreal_write (RState* r, rsexp port, rsexp obj)
{
    return r_port_printf (r, port, "%f", floreal_value (obj));
}

static rsexp complex_write (RState* r, rsexp port, rsexp obj)
{
    ensure (r_port_write (r, port, complex_real (obj)));

    if (!r_zero_p (complex_imag (obj))) {
        if (r_sign (complex_imag (obj)) > 0)
            ensure (r_port_write_char (r, port, '+'));

        ensure (r_port_write (r, port, complex_imag (obj)));
        ensure (r_port_write_char (r, port, 'i'));
    }

    return R_UNSPECIFIED;
}

static void complex_mark (RState* r, rsexp obj)
{
    r_gc_mark (r, complex_real (obj));
    r_gc_mark (r, complex_imag (obj));
}

static rbool fixreal_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    return mpq_cmp (fixreal_value (lhs), fixreal_value (rhs)) == 0;
}

static rbool floreal_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    return floreal_value (lhs) == floreal_value (rhs);
}

static rbool complex_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    return r_eqv_p (r, complex_real (lhs), complex_real (rhs))
        && r_eqv_p (r, complex_imag (lhs), complex_imag (rhs));
}

static void fixreal_finalize (RState* r, RObject* obj)
{
    mpq_clear (r_cast (RFixreal*, obj)->value);
}

static inline rsexp complex_new (RState* r,
                                 RTypeTag tag,
                                 rsexp real,
                                 rsexp imag)
{
    RComplex* obj;

    if (r_zero_p (imag))
        return real;

    obj = r_object_new (r, RComplex, tag);

    if (obj == NULL)
        return R_FAILURE;

    obj->real = real;
    obj->imag = imag;

    return complex_to_sexp (obj);
}

static inline rsexp fixcomplex_new (RState* r, rsexp real, rsexp imag)
{
    if (!r_exact_p (imag)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, imag);
        return R_FAILURE;
    }

    return complex_new (r, R_TAG_FIX_COMPLEX, real, imag);
}

static inline rsexp flocomplex_new (RState* r, rsexp real, rsexp imag)
{
    if (!r_inexact_p (imag)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, imag);
        return R_FAILURE;
    }

    return complex_new (r, R_TAG_FLO_COMPLEX, real, imag);
}

static inline rsexp floreal_to_fixreal (RState* r, rsexp n)
{
    mpq_t real;
    rsexp res;

    mpq_init (real);
    mpq_set_d (real, floreal_value (n));
    res = r_fixreal_new (r, real);
    mpq_clear (real);

    return res;
}

static inline rsexp inexact_to_exact (RState* r, rsexp n)
{
    rsexp real;
    rsexp imag;

    if (r_floreal_p (n))
        return floreal_to_fixreal (r, n);

    ensure (real = floreal_to_fixreal (r, complex_real (n)));
    ensure (imag = floreal_to_fixreal (r, complex_real (n)));

    return r_complex_new (r, real, imag);
}

static inline rsexp fixreal_to_floreal (RState* r, rsexp n)
{
    return r_floreal_new (r, mpq_get_d (fixreal_value (n)));
}

static inline rsexp exact_to_inexact (RState* r, rsexp n)
{
    rsexp real;
    rsexp imag;

    if (r_small_int_p (n))
        return smi_to_floreal (r, n);

    if (r_fixreal_p (n))
        return fixreal_to_floreal (r, n);

    ensure (real = fixreal_to_floreal (r, complex_real (n)));
    ensure (imag = fixreal_to_floreal (r, complex_imag (n)));

    return r_complex_new (r, real, imag);
}

rsexp try_small_int (mpq_t n)
{
    rint smi;

    mpq_canonicalize (n);

    /* If the denominator is not 1... */
    if (mpz_cmp_ui (mpq_denref (n), 1u) != 0)
        return R_FALSE;

    /* If the number is too large (to fit into a signed int)... */
    if (!mpz_fits_sint_p (mpq_numref (n)))
        return R_FALSE;

    smi = mpz_get_si (mpq_numref (n));

    /* If the number doesn't fit into the range of small integers... */
    if (smi > R_SMI_MAX || smi < R_SMI_MIN)
        return R_FALSE;

    return r_int_to_sexp (smi);
}

rsexp try_small_int_si (rint num, rint den)
{
    if (num % den != 0)
        return R_FALSE;

    num /= den;

    if (num > R_SMI_MAX || num < R_SMI_MIN)
        return R_FALSE;

    return r_int_to_sexp (num);
}

rsexp try_small_int_ui (ruint num, ruint den)
{
    if (num % den != 0)
        return R_FALSE;

    num /= den;

    if (num > R_SMI_MAX)
        return R_FALSE;

    return r_uint_to_sexp (num);
}

rsexp fixreal_normalize (rsexp n)
{
    rsexp res = try_small_int (fixreal_value (n));
    return r_false_p (res) ? n : res;
}

rsexp smi_to_fixreal (RState* r, rsexp n)
{
    RFixreal* obj = r_object_new (r, RFixreal, R_TAG_FIXREAL);

    if (obj == NULL)
        return R_FAILURE;

    mpq_init (obj->value);
    mpq_set_si (obj->value, r_int_from_sexp (n), 1);

    return fixreal_to_sexp (obj);
}

rsexp smi_to_floreal (RState* r, rsexp n)
{
    double real = r_cast (double, r_int_from_sexp (n));
    return r_floreal_new (r, real);
}

rsexp r_fixreal_new (RState* r, mpq_t value)
{
    rsexp smi;
    RFixreal* obj;

    smi = try_small_int (value);

    if (!r_false_p (smi))
        return smi;

    obj = r_object_new (r, RFixreal, R_TAG_FIXREAL);

    if (obj == NULL)
        return R_FAILURE;

    mpq_init (obj->value);
    mpq_set (obj->value, value);

    return fixreal_to_sexp (obj);
}

rsexp r_fixreal_new_si (RState* r, rint num, rint den)
{
    rsexp smi;
    RFixreal* obj;

    smi = try_small_int_si (num, den);

    if (!r_false_p (smi))
        return smi;

    obj = r_object_new (r, RFixreal, R_TAG_FIXREAL);

    if (obj == NULL)
        return R_FAILURE;

    mpq_init (obj->value);
    mpq_set_si (obj->value, num, den);
    mpq_canonicalize (obj->value);

    return fixreal_to_sexp (obj);
}

rsexp r_fixreal_new_ui (RState* r, ruint num, ruint den)
{
    rsexp smi;
    RFixreal* obj;

    smi = try_small_int_ui (num, den);

    if (!r_false_p (smi))
        return smi;

    obj = r_object_new (r, RFixreal, R_TAG_FIXREAL);

    if (obj == NULL)
        return R_FAILURE;

    mpq_init (obj->value);
    mpq_set_ui (obj->value, num, den);
    mpq_canonicalize (obj->value);

    return fixreal_to_sexp (obj);
}

rsexp r_floreal_new (RState* r, double value)
{
    RFloreal* obj = r_object_new (r, RFloreal, R_TAG_FLOREAL);

    if (obj == NULL)
        return R_FAILURE;

    obj->value = value;

    return fixreal_to_sexp (obj);
}

rsexp r_complex_new (RState* r, rsexp real, rsexp imag)
{
    if (r_exact_p (real))
        return fixcomplex_new (r, real, imag);

    if (r_inexact_p (real))
        return flocomplex_new (r, real, imag);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, real);
    return R_FAILURE;
}

rbool r_zero_p (rsexp n)
{
    if (r_small_int_p (n))
        return n == R_ZERO;

    if (r_floreal_p (n))
        return floreal_value (n) == 0.;

    if (r_fixreal_p (n))
        return mpq_cmp_si (fixreal_value (n), 0, 1) == 0;

    assert (r_complex_p (n) && "argument should be a number");

    return FALSE;
}

rbool r_one_p (rsexp n)
{
    if (r_small_int_p (n))
        return n == R_ONE;

    if (r_floreal_p (n))
        return floreal_value (n) == 1.;

    if (r_fixreal_p (n))
        return mpq_cmp_si (fixreal_value (n), 1, 1) == 0;

    assert (r_complex_p (n) && "argument should be a number");

    return FALSE;
}

rint r_sign (rsexp n)
{
    if (r_small_int_p (n))
        return r_int_from_sexp (n) >= 0 ? 1 : -1;

    if (r_fixreal_p (n))
        return mpq_cmp_si (fixreal_value (n), 0, 1) >= 0 ? 1 : -1;

    assert (r_floreal_p (n));
    return floreal_value (n) >= 0. ? 1 : -1;
}

rsexp r_real_part (RState* r, rsexp n)
{
    if (r_real_p (n))
        return n;

    if (r_complex_p (n))
        return complex_real (n);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, n);
    return R_FAILURE;
}

rsexp r_imag_part (RState* r, rsexp n)
{
    if (r_real_p (n))
        return r_exact_p (n) ? R_ZERO : r->flo_zero;

    if (r_complex_p (n))
        return complex_imag (n);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, n);
    return R_FAILURE;
}

rsexp r_exact_to_inexact (RState* r, rsexp n)
{
    if (r_inexact_p (n))
        return n;

    if (r_exact_p (n))
        return exact_to_inexact (r, n);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, n);
    return R_FAILURE;
}

rsexp r_inexact_to_exact (RState* r, rsexp n)
{
    if (r_exact_p (n))
        return n;

    if (r_inexact_p (n))
        return inexact_to_exact (r, n);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, n);
    return R_FAILURE;
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

rbool r_byte_p (rsexp obj)
{
    if (!r_small_int_p (obj))
        return FALSE;

    rint i = r_int_from_sexp (obj);

    return i >= 0 && i <= 255;
}

rbool r_integer_p (rsexp obj)
{
    if (r_small_int_p (obj))
        return TRUE;

    if (r_fixreal_p (obj))
        return mpz_cmp_si (mpq_denref (fixreal_value (obj)), 1) == 0;

    if (r_floreal_p (obj))
        return r_ceil (floreal_value (obj)) == floreal_value (obj);

    return FALSE;
}

rbool r_real_p (rsexp obj)
{
    return r_small_int_p (obj)
        || r_fixreal_p (obj)
        || r_floreal_p (obj);
}

rbool r_exact_p (rsexp obj)
{
    return r_small_int_p (obj)
        || r_fixreal_p (obj)
        || r_fixcomplex_p (obj);
}

rbool r_inexact_p (rsexp obj)
{
    return r_floreal_p (obj)
        || r_flocomplex_p (obj);
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

RTypeInfo fixreal_type = {
    .size = sizeof (RFixreal),
    .name = "fixreal",
    .ops = {
        .write = fixreal_write,
        .display = fixreal_write,
        .eqv_p = fixreal_eqv_p,
        .equal_p = fixreal_eqv_p,
        .finalize = fixreal_finalize
    }
};

RTypeInfo floreal_type = {
    .size = sizeof (RFloreal),
    .name = "floreal",
    .ops = {
        .write = floreal_write,
        .display = floreal_write,
        .eqv_p = floreal_eqv_p,
        .equal_p = floreal_eqv_p
    }
};

RTypeInfo complex_type = {
    .size = sizeof (RComplex),
    .name = "complex",
    .ops = {
        .write = complex_write,
        .display = complex_write,
        .eqv_p = complex_eqv_p,
        .equal_p = complex_eqv_p,
        .mark = complex_mark
    }
};
