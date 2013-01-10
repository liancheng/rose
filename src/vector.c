#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/vector.h"

#include <assert.h>
#include <stdarg.h>

typedef struct RVector RVector;

struct RVector {
    R_OBJECT_HEADER
    rsize  length;
    rsexp* data;
};

#define vector_from_sexp(obj)   (r_cast (RVector*, (obj)))
#define vector_to_sexp(vector)  (r_cast (rsexp, (vector)))

typedef RWriteFunc ROutputFunc;

static rsexp vector_output (RState* r,
                            rsexp port,
                            rsexp obj,
                            ROutputFunc output_fn)
{
    rsize i;
    rsize length;

    length = r_uint_from_sexp (r_vector_length (obj));
    ensure (r_port_puts (r, port, "#("));

    if (length) {
        ensure (output_fn (r, port, r_vector_ref (r, obj, 0)));

        for (i = 1u; i < length; ++i) {
            ensure (r_port_puts (r, port, " "));
            ensure (output_fn (r, port, r_vector_ref (r, obj, i)));
        }
    }

    ensure (r_port_puts (r, port, ")"));

    return R_UNSPECIFIED;
}

static rsexp vector_write (RState* r, rsexp port, rsexp obj)
{
    return vector_output (r, port, obj, r_port_write);
}

static rsexp vector_display (RState* r, rsexp port, rsexp obj)
{
    return vector_output (r, port, obj, r_port_display);
}

static void vector_finalize (RState* r, RObject* obj)
{
    RVector* v = r_cast (RVector*, obj);
    r_free (r, v->data);
}

static rsexp vvector (RState* r, rsize k, va_list args)
{
    rsize i;
    rsexp res = r_vector_new (r, k, R_UNDEFINED);

    for (i = 0; i < k; ++i)
        r_vector_set_x (r, res, i, va_arg (args, rsexp));

    return res;
}

static rbool vector_equal_p (RState* r, rsexp lhs, rsexp rhs)
{
    rsize k;
    rsize lhs_len;
    rsize rhs_len;
    rsexp* lhs_data;
    rsexp* rhs_data;

    if (!r_vector_p (lhs) || !r_vector_p (rhs))
        return FALSE;

    lhs_len = vector_from_sexp (lhs)->length;
    rhs_len = vector_from_sexp (rhs)->length;

    if (lhs_len != rhs_len)
        return FALSE;

    lhs_data = vector_from_sexp (lhs)->data;
    rhs_data = vector_from_sexp (rhs)->data;

    for (k = 0; k < lhs_len; ++k)
        if (!r_equal_p (r, lhs_data [k], rhs_data [k]))
            return FALSE;

    return TRUE;
}

static rbool check_index_overflow (RState* r, rsexp vec, rsize k)
{
    rsize length = r_uint_from_sexp (r_vector_length (vec));

    if (k >= length) {
        r_error_code (r, R_ERR_INDEX_OVERFLOW);
        return FALSE;
    }

    return TRUE;
}

static void vector_mark (RState* r, rsexp obj)
{
    rsize i;
    rsize length = r_uint_from_sexp (r_vector_length (obj));

    for (i = 0; i < length; ++i)
        r_gc_mark (r, r_vector_ref (r, obj, i));
}

rsexp r_vector_new (RState* r, rsize k, rsexp fill)
{
    rsize i;
    RVector* res = r_object_new (r, RVector, R_TAG_VECTOR);

    if (!res)
        return R_FAILURE;

    res->length = k;
    res->data   = k ? r_new_array (r, rsexp, k) : NULL;

    if (k && !res->data) {
        r_free (r, res);
        return R_FAILURE;
    }

    for (i = 0; i < k; ++i)
        res->data [i] = fill;

    return vector_to_sexp (res);
}

rsexp r_vector (RState* r, rsize k, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, k);
    res = vvector (r, k, args);
    va_end (args);

    return res;
}

rbool r_vector_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_VECTOR;
}

rsexp r_vector_ref (RState* r, rsexp vector, rsize k)
{
    return check_index_overflow (r, vector, k)
           ? vector_from_sexp (vector)->data [k]
           : R_FAILURE;
}

rsexp r_vector_set_x (RState* r, rsexp vector, rsize k, rsexp obj)
{
    if (!check_index_overflow (r, vector, k))
        return R_FAILURE;

    vector_from_sexp (vector)->data [k] = obj;

    return R_TRUE;
}

rsexp r_vector_length (rsexp vector)
{
    return r_uint_to_sexp (vector_from_sexp (vector)->length);
}

rsexp r_list_to_vector (RState* r, rsexp list)
{
    rsexp length, res;
    rsize i;

    ensure_or_goto (length = r_length (r, list), exit);
    ensure_or_goto (res = r_vector_new (r,
                                        r_uint_from_sexp (length),
                                        R_UNDEFINED),
                    exit);

    for (i = 0; !r_null_p (list); ++i) {
        ensure_or_goto (r_vector_set_x (r, res, i, r_car (list)), exit);
        list = r_cdr (list);
    }

exit:
    return res;
}

rsexp r_vector_to_list (RState* r, rsexp vector)
{
    rsize i;
    rsexp datum, res;
    rsize length = r_uint_from_sexp (r_vector_length (vector));

    r_gc_scope_open (r);

    for (i = length, res = R_NULL; i > 0; --i) {
        ensure_or_goto (datum = r_vector_ref (r, vector, i - 1), exit);
        ensure_or_goto (res = r_cons (r, datum, res), exit);
    }

exit:
    r_gc_scope_close_and_protect (r, res);

    return res;
}

RTypeInfo vector_type = {
    .size = sizeof (RVector),
    .name = "vector",
    .ops = {
        .write = vector_write,
        .display = vector_display,
        .equal_p = vector_equal_p,
        .mark = vector_mark,
        .finalize = vector_finalize
    }
};
