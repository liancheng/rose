#include "boxed.h"

#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/vector.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

#define SEXP_TO_VECTOR(s) R_BOXED_VALUE(s).vector

rboolean r_vector_p(rsexp sexp)
{
    return r_boxed_get_type(sexp) == SEXP_VECTOR;
}

rsexp r_vector_new(rsize k)
{
    rsexp res;

    res = (rsexp)GC_NEW(RBoxed);
    SEXP_TO_VECTOR(res).size = k;
    SEXP_TO_VECTOR(res).data = k ? GC_MALLOC(k * sizeof(rsexp)) : NULL;

    r_boxed_set_type(res, SEXP_VECTOR);

    return res;
}

rsexp r_make_vector(rsize k, rsexp fill)
{
    rsexp  res;
    rsexp* data;
    rsize  i;

    res  = r_vector_new(k);
    data = SEXP_TO_VECTOR(res).data;

    for (i = 0; i < k; ++i)
        data[i] = fill;

    return res;
}

rsexp r_vector(rsize k, ...)
{
    va_list args;
    rsize   i;
    rsexp   res;
    rsexp*  data;

    va_start(args, k);

    res  = r_vector_new(k);
    data = SEXP_TO_VECTOR(res).data;

    for (i = 0; i < k; ++i)
        data[i] = va_arg(args, rsexp);

    va_end(args);

    return res;
}

rsexp r_vector_ref(rsexp vector, rsize k)
{
    assert(r_vector_p(vector));
    assert(SEXP_TO_VECTOR(vector).size > k);
    return SEXP_TO_VECTOR(vector).data[k];
}

rsexp r_vector_set_x(rsexp vector, rsize k, rsexp obj)
{
    assert(r_vector_p(vector));
    assert(SEXP_TO_VECTOR(vector).size > k);

    SEXP_TO_VECTOR(vector).data[k] = obj;

    return R_SEXP_UNSPECIFIED;
}

rsize r_vector_length(rsexp vector)
{
    assert(r_vector_p(vector));
    return SEXP_TO_VECTOR(vector).size;
}

rsexp r_list_to_vector(rsexp list)
{
    rsize length = r_length(list);
    rsexp res = r_make_vector(length, R_SEXP_UNSPECIFIED);
    rsize k;

    for (k = 0; k < length; ++k) {
        r_vector_set_x(res, k, r_car(list));
        list = r_cdr(list);
    }

    return res;
}

rboolean r_vector_equal_p(rsexp lhs, rsexp rhs)
{
    rsize size;
    rsize k;

    if (!r_vector_p(rhs))
        return FALSE;

    size = SEXP_TO_VECTOR(lhs).size;

    if (SEXP_TO_VECTOR(rhs).size != size)
        return FALSE;

    rsexp* lhs_data = SEXP_TO_VECTOR(lhs).data;
    rsexp* rhs_data = SEXP_TO_VECTOR(rhs).data;

    for (k = 0; k < size; ++k)
        if (!r_equal_p(lhs_data[k], rhs_data[k]))
            return FALSE;

    return TRUE;
}

void r_write_vector(FILE* output, rsexp sexp, rsexp context)
{
    rsize i;
    rsize length;

    assert(r_vector_p(sexp));

    fprintf(output, "#(");

    length = r_vector_length(sexp);

    if (length) {
        r_write(output, r_vector_ref(sexp, 0), context);

        for (i = 1; i < length; ++i) {
            fprintf(output, " ");
            r_write(output, r_vector_ref(sexp, i), context);
        }
    }

    fprintf(output, ")");
}
