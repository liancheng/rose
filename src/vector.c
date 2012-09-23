#include "detail/sexp.h"
#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/vector.h"
#include "rose/writer.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

struct RVector {
    RType* type;
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

static RType* vector_type_info ()
{
    static RType type = {
        .size    = sizeof (RVector),
        .name    = "vector",
        .write   = write_vector,
        .display = display_vector
    };

    return &type;
}

static rsexp vvector (rsize k, va_list args)
{
    return r_list_to_vector (r_vlist (k, args));
}

rsexp r_vector_new (rsize k, rsexp fill)
{
    RVector* res = GC_NEW (RVector);

    res->type   = vector_type_info ();
    res->length = k;
    res->data   = k ? GC_MALLOC (k * sizeof (rsexp)) : NULL;

    while (k--)
        res->data [k] = fill;

    return VECTOR_TO_SEXP (res);
}

rsexp r_vector (rsize k, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, k);
    res = vvector (k, args);
    va_end (args);

    return res;
}

rbool r_vector_p (rsexp obj)
{
    return r_boxed_p (obj) &&
           (R_SEXP_TYPE (obj) == vector_type_info ());
}

rbool r_vector_equal_p (rsexp lhs, rsexp rhs)
{
    rsize length;
    rsize k;

    if (!r_vector_p (rhs))
        return FALSE;

    length = VECTOR_FROM_SEXP (lhs)->length;

    if (VECTOR_FROM_SEXP (rhs)->length != length)
        return FALSE;

    rsexp* lhs_data = VECTOR_FROM_SEXP (lhs)->data;
    rsexp* rhs_data = VECTOR_FROM_SEXP (rhs)->data;

    for (k = 0; k < length; ++k)
        if (!r_equal_p (lhs_data [k], rhs_data [k]))
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

rsexp r_list_to_vector (rsexp list)
{
    rsexp res = r_vector_new (r_length (list), R_UNSPECIFIED);
    rsize k   = 0u;

    while (!r_null_p (list)) {
        r_vector_set_x (res, k++, r_car (list));
        list = r_cdr (list);
    }

    return res;
}
