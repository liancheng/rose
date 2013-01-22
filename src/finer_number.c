#include "detail/finer_number.h"
#include "detail/gc.h"
#include "detail/io.h"
#include "detail/number.h"
#include "detail/state.h"
#include "rose/eq.h"
#include "rose/error.h"

#include <assert.h>

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

static rsexp write_floreal (RState* r, rsexp port, rsexp obj)
{
    return r_port_printf (r, port, "%f", floreal_value (obj));
}

static rsexp write_complex (RState* r, rsexp port, rsexp obj)
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

rsexp smi_to_fixreal (RState* r, rsexp n)
{
    assert (r_small_int_p (n));
    return r_fixreal_new_si (r, r_int_from_sexp (n), 1);
}

rsexp r_fixreal_new (RState* r, mpq_t value)
{
    RFixreal* obj = r_object_new (r, RFixreal, R_TAG_FIXREAL);

    if (obj == NULL)
        return R_FAILURE;

    mpq_init (obj->value);
    mpq_set (obj->value, value);
    mpq_canonicalize (obj->value);

    return fixreal_to_sexp (obj);
}

rsexp r_fixreal_new_si (RState* r, rint num, rint den)
{
    RFixreal* obj = r_object_new (r, RFixreal, R_TAG_FIXREAL);

    if (obj == NULL)
        return R_FAILURE;

    mpq_init (obj->value);
    mpq_set_si (obj->value, num, den);
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
    RComplex* obj;

     if (r_exact_p (real) && !r_exact_p (imag)) {
         r_error_code (r, R_ERR_WRONG_TYPE_ARG, imag);
         return R_FAILURE;
     }

     if (r_inexact_p (real) && !r_inexact_p (imag)) {
         r_error_code (r, R_ERR_WRONG_TYPE_ARG, imag);
         return R_FAILURE;
     }

    if (r_zero_p (imag))
        return real;

    obj = r_object_new (r, RComplex, R_TAG_COMPLEX);

    if (obj == NULL)
        return R_FAILURE;

    obj->real = real;
    obj->imag = imag;

    return complex_to_sexp (obj);
}

rbool r_fixreal_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_FIXREAL;
}

rbool r_floreal_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_FLOREAL;
}

rbool r_complex_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_COMPLEX;
}

rbool r_zero_p (rsexp n)
{
    assert (r_number_p (n));

    if (r_small_int_p (n))
        return n == R_ZERO;

    if (r_floreal_p (n))
        return floreal_value (n) == 0.;

    if (r_fixreal_p (n))
        return mpq_cmp_si (fixreal_value (n), 0, 1) == 0;

    return FALSE;
}

rint r_sign (rsexp n)
{
    assert (r_real_p (n));

    if (r_small_int_p (n))
        return r_int_from_sexp (n) >= 0 ? 1 : -1;

    if (r_fixreal_p (n))
        return mpq_cmp_si (fixreal_value (n), 0, 1) >= 0 ? 1 : -1;

    return floreal_value (n) >= 0. ? 1 : -1;
}

rsexp r_real_part (RState* r, rsexp n)
{
    if (r_real_p (n))
        return n;

    if (r_complex_p (n))
        return complex_real (n);

    r_error_code (r, R_ERR_WRONG_TYPE_ARG, n);

    return FALSE;
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

RTypeInfo floreal_type = {
    .size = sizeof (RFloreal),
    .name = "floreal",
    .ops = {
        .write = write_floreal,
        .display = write_floreal,
        .eqv_p = floreal_eqv_p,
        .equal_p = floreal_eqv_p
    }
};

RTypeInfo complex_type = {
    .size = sizeof (RComplex),
    .name = "complex",
    .ops = {
        .write = write_complex,
        .display = write_complex,
        .eqv_p = complex_eqv_p,
        .equal_p = complex_eqv_p,
        .mark = complex_mark
    }
};
