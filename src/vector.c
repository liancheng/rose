#include "rose/pair.h"
#include "rose/vector.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

#define SEXP_TO_VECTOR(s) (((r_boxed*)s)->as.vector)

r_sexp sexp_make_vector(size_t k, r_sexp fill)
{
    r_sexp res = (r_sexp)GC_MALLOC(sizeof(r_boxed));
    SEXP_TYPE(res) = SEXP_VECTOR;

    SEXP_TO_VECTOR(res).size     = k;
    SEXP_TO_VECTOR(res).capacity = k;
    SEXP_TO_VECTOR(res).data     = k ? GC_MALLOC(k * sizeof(r_sexp)) : NULL;

    for (size_t i = 0; i < k; ++i)
        SEXP_TO_VECTOR(res).data[i] = fill;

    return res;
}

r_sexp sexp_vector(size_t k, ...)
{
    r_sexp acc = SEXP_NULL;

    va_list args;
    va_start(args, k);

    for (size_t i = 0; i < k; ++i)
        acc = sexp_cons(va_arg(args, r_sexp), acc);

    va_end(args);

    return sexp_list_to_vector(sexp_reverse(acc));
}

r_sexp sexp_vector_ref(r_sexp vector, int k)
{
    assert(SEXP_VECTOR_P(vector));
    assert(SEXP_TO_VECTOR(vector).size > k);
    return SEXP_TO_VECTOR(vector).data[k];
}

r_sexp sexp_vector_set_x(r_sexp vector, int k, r_sexp obj)
{
    assert(SEXP_VECTOR_P(vector));
    assert(SEXP_TO_VECTOR(vector).size > k);
    SEXP_TO_VECTOR(vector).data[k] = obj;
    return SEXP_UNSPECIFIED;
}

size_t sexp_vector_length(r_sexp vector)
{
    assert(SEXP_VECTOR_P(vector));
    return SEXP_TO_VECTOR(vector).size;
}

r_sexp sexp_list_to_vector(r_sexp list)
{
    size_t length = sexp_length(list);
    r_sexp res = sexp_make_vector(length, SEXP_UNSPECIFIED);

    for (size_t k = 0; k < length; ++k) {
        sexp_vector_set_x(res, k, sexp_car(list));
        list = sexp_cdr(list);
    }

    return res;
}
