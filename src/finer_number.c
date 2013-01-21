#include "detail/finer_number.h"
#include "detail/gc.h"
#include "detail/io.h"
#include "detail/number.h"
#include "rose/eq.h"
#include "rose/error.h"

#include <assert.h>

static const RFloreal flo_zero = {
    STATIC_OBJECT_HEADER (R_TAG_STRING)
    .value = 0.
};

#define R_FLO_ZERO (floreal_to_sexp (&flo_zero))

static const RFloreal inexact_one = {
    STATIC_OBJECT_HEADER (R_TAG_STRING)
    .value = 1.
};

#define R_FLO_ONE (floreal_to_sexp (&flo_one))

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
    return r_port_printf (r, port, "%f", floreal_from_sexp (obj)->value);
}

static rsexp write_complex (RState* r, rsexp port, rsexp obj)
{
    RComplex* n = complex_from_sexp (obj);

    ensure (r_port_write (r, port, n->real));

    if (!r_zero_p (n->imag)) {
        if (r_sign (n->imag) > 0)
            ensure (r_port_write_char (r, port, '+'));

        ensure (r_port_write (r, port, n->imag));
        ensure (r_port_write_char (r, port, 'i'));
    }

    return R_UNSPECIFIED;
}

static rbool fixreal_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    RFixreal* lhs_num = fixreal_from_sexp (lhs);
    RFixreal* rhs_num = fixreal_from_sexp (rhs);

    return mpq_cmp (lhs_num->value, rhs_num->value) == 0;
}

static rbool floreal_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    RFloreal* lhs_num = floreal_from_sexp (lhs);
    RFloreal* rhs_num = floreal_from_sexp (rhs);

    return lhs_num->value == rhs_num->value;
}

static rbool complex_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    RComplex* lhs_num = complex_from_sexp (lhs);
    RComplex* rhs_num = complex_from_sexp (rhs);

    return r_eqv_p (r, lhs_num->real, rhs_num->real)
        && r_eqv_p (r, lhs_num->imag, rhs_num->imag);
}

static void fixreal_finalize (RState* r, RObject* obj)
{
    RFixreal* n = fixreal_from_sexp (obj);
    mpq_clear (n->value);
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
        return floreal_from_sexp (n)->value == 0.;

    if (r_fixreal_p (n))
        return mpq_cmp_si (fixreal_from_sexp (n)->value, 0, 1) == 0;

    return FALSE;
}

rint r_sign (rsexp n)
{
    assert (r_real_p (n));

    if (r_small_int_p (n))
        return r_int_from_sexp (n) >= 0 ? 1 : -1;

    if (r_fixreal_p (n))
        return mpq_cmp_si (fixreal_from_sexp (n)->value, 0, 1) >= 0 ? 1 : -1;

    if (r_floreal_p (n))
        return floreal_from_sexp (n)->value >= 0. ? 1 : -1;

    assert (0);

    return FALSE;
}

rsexp r_real_part (rsexp n)
{
    if (r_real_p (n))
        return n;

    if (r_complex_p (n))
        return complex_from_sexp (n)->real;

    assert (0);

    return FALSE;
}

rsexp r_imag_part (rsexp n)
{
    if (r_real_p (n))
        return r_exact_p (n) ? R_ZERO : R_FLO_ZERO;

    if (r_complex_p (n))
        return complex_from_sexp (n)->imag;

    assert (0);

    return FALSE;
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
        .equal_p = complex_eqv_p
    }
};
