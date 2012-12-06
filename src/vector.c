#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/eq.h"
#include "rose/memory.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/vector.h"

#include <assert.h>
#include <stdarg.h>

struct RVector {
    R_OBJECT_HEADER
    rsize  length;
    rsexp* data;
};

#define vector_from_sexp(obj)   ((RVector*) (obj))
#define vector_to_sexp(vector)  ((rsexp) (vector))

typedef void (*ROutputFunc) (RState* state, rsexp, rsexp);

static void output_vector (RState*     state,
                           rsexp       port,
                           rsexp       obj,
                           ROutputFunc output_fn)
{
    assert (r_vector_p (obj));

    rsize i;
    rsize length = r_vector_length (obj);

    r_port_puts (port, "#(");

    if (length) {
        output_fn (state, port, r_vector_ref (obj, 0));

        for (i = 1; i < length; ++i) {
            r_port_puts (port, " ");
            output_fn (state, port, r_vector_ref (obj, i));
        }
    }

    r_port_puts (port, ")");
}

static void write_vector (RState* state, rsexp port, rsexp obj)
{
    output_vector (state, port, obj, r_port_write);
}

static void display_vector (RState* state, rsexp port, rsexp obj)
{
    output_vector (state, port, obj, r_port_display);
}

static void destruct_vector (RState* state, RObject* obj)
{
    RVector* v = r_cast (RVector*, obj);
    r_free (state, v->data);
}

static rsexp vvector (RState* state, rsize k, va_list args)
{
    rsize i;
    rsexp res = r_vector_new (state, k, R_FALSE);

    for (i = 0; i < k; ++i)
        r_vector_set_x (res, i, va_arg (args, rsexp));

    return res;
}

static rbool vector_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    rsize  k;
    rsize  lhs_len;
    rsize  rhs_len;
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
        if (!r_equal_p (state, lhs_data [k], rhs_data [k]))
            return FALSE;

    return TRUE;
}

void init_vector_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RVector);
    type->name         = "vector";
    type->ops.write    = write_vector;
    type->ops.display  = display_vector;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = vector_equal_p;
    type->ops.mark     = NULL;
    type->ops.destruct = destruct_vector;

    state->builtin_types [R_VECTOR_TAG] = type;
}

rsexp r_vector_new (RState* state, rsize k, rsexp fill)
{
    rsize i;
    RVector* res = r_object_new (state, RVector, R_VECTOR_TAG);

    res->length = k;
    res->data = k ? r_alloc (state, k * sizeof (rsexp)) : NULL;

    for (i = 0; i < k; ++i)
        res->data [i] = fill;

    return vector_to_sexp (res);
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
    return r_type_tag (obj) == R_VECTOR_TAG;
}

rsexp r_vector_ref (rsexp vector, rsize k)
{
    assert (r_vector_p (vector));
    assert (r_vector_length (vector) > k);
    return vector_from_sexp (vector)->data [k];
}

rsexp r_vector_set_x (rsexp vector, rsize k, rsexp obj)
{
    assert (r_vector_p (vector));
    assert (vector_from_sexp (vector)->length > k);

    vector_from_sexp (vector)->data [k] = obj;

    return R_UNSPECIFIED;
}

rsize r_vector_length (rsexp vector)
{
    assert (r_vector_p (vector));
    return vector_from_sexp (vector)->length;
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
