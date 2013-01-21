#include "detail/finer_number.h"
#include "detail/gc.h"
#include "detail/io.h"
#include "detail/number.h"
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
    return r_port_printf (r, port, "%f", floreal_from_sexp (obj)->value);
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
