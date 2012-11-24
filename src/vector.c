#include "detail/sexp.h"
#include "rose/eq.h"
#include "rose/memory.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/vector.h"
#include "rose/writer.h"

#include <assert.h>
#include <stdarg.h>

struct RVector {
    R_OBJECT_HEADER
    rsize  length;
    rsexp* data;
};

#define VECTOR_FROM_SEXP(obj)   ((RVector*) (obj))
#define VECTOR_TO_SEXP(vector)  ((rsexp) (vector))

typedef void (*ROutputFunction) (rsexp, rsexp);

static void output_vector (rsexp           port,
                           rsexp           obj,
                           ROutputFunction output_fn)
{
    assert (r_vector_p (obj));

    rsize i;
    rsize length = r_vector_length (obj);

    r_port_puts (port, "#(");

    if (length) {
        output_fn (port, r_vector_ref (obj, 0));

        for (i = 1; i < length; ++i) {
            r_port_puts (port, " ");
            output_fn (port, r_vector_ref (obj, i));
        }
    }

    r_port_puts (port, ")");
}

static void write_vector (rsexp port, rsexp obj)
{
    output_vector (port, obj, r_write);
}

static void display_vector (rsexp port, rsexp obj)
{
    output_vector (port, obj, r_display);
}

static void destruct_vector (RState* state, RObject* obj)
{
    RVector* v = r_cast (RVector*, obj);
    r_free (state, v->data);
}

static RTypeDescriptor* vector_type_info ()
{
    static RTypeDescriptor type = {
        .size = sizeof (RVector),
        .name = "vector",
        .ops = {
            .write = write_vector,
            .display = display_vector,
            .eqv_p = NULL,
            .equal_p = r_vector_equal_p,
            .mark = NULL,
            .destruct = destruct_vector
        }
    };

    return &type;
}

static rsexp vvector (RState* state, rsize k, va_list args)
{
    rsexp res;
    rsize i;

    res = r_vector_new (state, k, R_FALSE);

    for (i = 0; i < k; ++i)
        r_vector_set_x (res, i, va_arg (args, rsexp));

    return res;
}

rsexp r_vector_new (RState* state, rsize k, rsexp fill)
{
    rsize i;
    RObject* obj = r_object_new (state, R_TYPE_VECTOR, vector_type_info ());
    RVector* res = r_cast (RVector*, obj);

    res->length = k;
    res->data = k ? r_alloc (state, k * sizeof (rsexp)) : NULL;

    for (i = 0; i < k; ++i)
        res->data [i] = fill;

    return VECTOR_TO_SEXP (res);
}

rsexp r_vector (RState* state, rsize k, ...)
{
    va_list args;
    rsexp res;

    va_start (args, k);
    res = vvector (state, k, args);
    va_end (args);

    return res;
}

rbool r_vector_p (rsexp obj)
{
    return r_type_tag (obj) == R_TYPE_VECTOR;
}

rbool r_vector_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    rsize  k;
    rsize  lhs_len;
    rsize  rhs_len;
    rsexp* lhs_data;
    rsexp* rhs_data;

    if (!r_vector_p (lhs) || !r_vector_p (rhs))
        return FALSE;

    lhs_len = VECTOR_FROM_SEXP (lhs)->length;
    rhs_len = VECTOR_FROM_SEXP (rhs)->length;

    if (lhs_len != rhs_len)
        return FALSE;

    lhs_data = VECTOR_FROM_SEXP (lhs)->data;
    rhs_data = VECTOR_FROM_SEXP (rhs)->data;

    for (k = 0; k < lhs_len; ++k)
        if (!r_equal_p (state, lhs_data [k], rhs_data [k]))
            return FALSE;

    return TRUE;
}

rsexp r_vector_ref (rsexp vector, rsize k)
{
    assert (r_vector_p (vector));
    assert (r_vector_length (vector) > k);
    return VECTOR_FROM_SEXP (vector)->data [k];
}

rsexp r_vector_set_x (rsexp vector, rsize k, rsexp obj)
{
    assert (r_vector_p (vector));
    assert (VECTOR_FROM_SEXP (vector)->length > k);

    VECTOR_FROM_SEXP (vector)->data [k] = obj;

    return R_UNSPECIFIED;
}

rsize r_vector_length (rsexp vector)
{
    assert (r_vector_p (vector));
    return VECTOR_FROM_SEXP (vector)->length;
}

rsexp r_list_to_vector (RState* state, rsexp list)
{
    rsexp res;
    rsize i;

    res = r_vector_new (state, r_length (list), R_UNSPECIFIED);

    for (i = 0; !r_null_p (list); ++i) {
        r_vector_set_x (res, i, r_car (list));
        list = r_cdr (list);
    }

    return res;
}

rsexp r_vector_to_list (RState* state, rsexp vector)
{
    rsize i;
    rsexp res;

    for (i = r_vector_length (vector), res = R_NULL; i > 0; --i)
        res = r_cons (state, r_vector_ref (vector, i - 1), res);

    return res;
}
