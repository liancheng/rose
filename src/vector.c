#include "rose/pair.h"
#include "rose/vector.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

rsexp r_vector_new(rsize k)
{
    rsexp res;

    res                      = (rsexp)GC_NEW(RBoxed);
    SEXP_TYPE(res)           = SEXP_VECTOR;
    SEXP_TO_VECTOR(res).size = k;
    SEXP_TO_VECTOR(res).data = k ? GC_MALLOC(k * sizeof(rsexp)) : NULL;

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
    assert(SEXP_VECTOR_P(vector));
    assert(SEXP_TO_VECTOR(vector).size > k);
    return SEXP_TO_VECTOR(vector).data[k];
}

rsexp r_vector_set_x(rsexp vector, rsize k, rsexp obj)
{
    assert(SEXP_VECTOR_P(vector));
    assert(SEXP_TO_VECTOR(vector).size > k);

    SEXP_TO_VECTOR(vector).data[k] = obj;

    return SEXP_UNSPECIFIED;
}

rsize r_vector_length(rsexp vector)
{
    assert(SEXP_VECTOR_P(vector));
    return SEXP_TO_VECTOR(vector).size;
}

rsexp r_list_to_vector(rsexp list)
{
    rsize length = r_length(list);
    rsexp res = r_make_vector(length, SEXP_UNSPECIFIED);
    rsize k;

    for (k = 0; k < length; ++k) {
        r_vector_set_x(res, k, r_car(list));
        list = r_cdr(list);
    }

    return res;
}
