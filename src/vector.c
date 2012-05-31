#include "cell.h"

#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/vector.h"
#include "rose/write.h"

#include <assert.h>
#include <stdarg.h>

#define SEXP_TO_VECTOR(s) R_CELL_VALUE (s).vector

rboolean r_vector_p (rsexp obj)
{
    return r_cell_p (obj) &&
           r_cell_get_type (obj) == SEXP_VECTOR;
}

rsexp r_vector_new (rsize k)
{
    R_SEXP_NEW (res, SEXP_VECTOR);

    SEXP_TO_VECTOR (res).length = k;
    SEXP_TO_VECTOR (res).data = k
                              ? GC_MALLOC (k * sizeof (rsexp))
                              : NULL;

    return res;
}

rsexp r_make_vector (rsize k, rsexp fill)
{
    rsexp  res;
    rsexp* data;
    rsize  i;

    res  = r_vector_new (k);
    data = SEXP_TO_VECTOR (res).data;

    for (i = 0; i < k; ++i)
        data[i] = fill;

    return res;
}

rsexp r_vector (rsize k, ...)
{
    va_list args;
    rsize   i;
    rsexp   res;
    rsexp*  data;

    res  = r_vector_new (k);
    data = SEXP_TO_VECTOR (res).data;

    va_start (args, k);

    for (i = 0; i < k; ++i)
        data[i] = va_arg (args, rsexp);

    va_end (args);

    return res;
}

rsexp r_vector_ref (rsexp vector, rsize k)
{
    assert (r_vector_p (vector));
    assert (SEXP_TO_VECTOR (vector).length > k);
    return SEXP_TO_VECTOR (vector).data[k];
}

rsexp r_vector_set_x (rsexp vector, rsize k, rsexp obj)
{
    assert (r_vector_p (vector));
    assert (SEXP_TO_VECTOR (vector).length > k);

    SEXP_TO_VECTOR (vector).data[k] = obj;

    return R_SEXP_UNSPECIFIED;
}

rsize r_vector_length (rsexp vector)
{
    assert (r_vector_p (vector));
    return SEXP_TO_VECTOR (vector).length;
}

rsexp r_list_to_vector (rsexp list)
{
    rsize length = r_length (list);
    rsexp res = r_make_vector (length, R_SEXP_UNSPECIFIED);
    rsize k;

    for (k = 0; k < length; ++k) {
        r_vector_set_x (res, k, r_car (list));
        list = r_cdr (list);
    }

    return res;
}

rboolean r_vector_equal_p (rsexp lhs, rsexp rhs)
{
    rsize length;
    rsize k;

    if (!r_vector_p (rhs))
        return FALSE;

    length = SEXP_TO_VECTOR (lhs).length;

    if (SEXP_TO_VECTOR (rhs).length != length)
        return FALSE;

    rsexp* lhs_data = SEXP_TO_VECTOR (lhs).data;
    rsexp* rhs_data = SEXP_TO_VECTOR (rhs).data;

    for (k = 0; k < length; ++k)
        if (!r_equal_p (lhs_data[k], rhs_data[k]))
            return FALSE;

    return TRUE;
}

typedef void (*ROutputFunction) (rsexp, rsexp, rsexp);

static void output_vector (rsexp           port,
                           rsexp           obj,
                           ROutputFunction output_fn,
                           rsexp           context)
{
    rsize i;
    rsize length;

    assert (r_vector_p (obj));

    r_port_puts (port, "#(");

    length = r_vector_length (obj);

    if (length) {
        output_fn (port, r_vector_ref (obj, 0), context);

        for (i = 1; i < length; ++i) {
            r_port_puts (port, " ");
            output_fn (port, r_vector_ref (obj, i), context);
        }
    }

    r_port_puts (port, ")");
}

void r_write_vector (rsexp port, rsexp obj, rsexp context)
{
    output_vector (port, obj, r_write, context);
}

void r_display_vector (rsexp port, rsexp obj, rsexp context)
{
    output_vector (port, obj, r_display, context);
}
