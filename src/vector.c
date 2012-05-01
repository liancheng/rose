#include "rose/pair.h"
#include "rose/vector.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

#define SEXP_TO_VECTOR(s) (((RBoxed*)s)->as.vector)

rsexp sexp_make_vector(rsize k, rsexp fill)
{
    rsexp res = (rsexp)GC_NEW(RBoxed);

    SEXP_TYPE(res)           = SEXP_VECTOR;
    SEXP_TO_VECTOR(res).size = k;
    SEXP_TO_VECTOR(res).data = k ? GC_MALLOC(k * sizeof(rsexp)) : NULL;

    for (rsize i = 0; i < k; ++i)
        SEXP_TO_VECTOR(res).data[i] = fill;

    return res;
}

rsexp sexp_vector(rsize k, ...)
{
    rsexp acc = SEXP_NULL;

    va_list args;
    va_start(args, k);

    for (rsize i = 0; i < k; ++i)
        acc = sexp_cons(va_arg(args, rsexp), acc);

    va_end(args);

    return sexp_list_to_vector(sexp_reverse(acc));
}

rsexp sexp_vector_ref(rsexp vector, rsize k)
{
    assert(SEXP_VECTOR_P(vector));
    assert(SEXP_TO_VECTOR(vector).size > k);
    return SEXP_TO_VECTOR(vector).data[k];
}

rsexp sexp_vector_set_x(rsexp vector, rsize k, rsexp obj)
{
    assert(SEXP_VECTOR_P(vector));
    assert(SEXP_TO_VECTOR(vector).size > k);

    SEXP_TO_VECTOR(vector).data[k] = obj;

    return SEXP_UNSPECIFIED;
}

rsize sexp_vector_length(rsexp vector)
{
    assert(SEXP_VECTOR_P(vector));
    return SEXP_TO_VECTOR(vector).size;
}

rsexp sexp_list_to_vector(rsexp list)
{
    rsize length = sexp_length(list);
    rsexp res = sexp_make_vector(length, SEXP_UNSPECIFIED);

    for (rsize k = 0; k < length; ++k) {
        sexp_vector_set_x(res, k, sexp_car(list));
        list = sexp_cdr(list);
    }

    return res;
}
