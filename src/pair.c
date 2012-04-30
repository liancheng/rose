#include "rose/pair.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdlib.h>
#include <stdarg.h>

#define SEXP_FROM_PAIR(p) (((r_sexp)(p) << 2) | SEXP_PAIR_TAG)
#define SEXP_TO_PAIR(s)   ((r_pair*)((s) >> 2))

r_sexp sexp_cons(r_sexp car, r_sexp cdr)
{
    r_pair* pair = GC_NEW(r_pair);
    pair->car = car;
    pair->cdr = cdr;
    return SEXP_FROM_PAIR(pair);
}

r_sexp sexp_car(r_sexp sexp)
{
    assert(SEXP_PAIR_P(sexp));
    return SEXP_TO_PAIR(sexp)->car;
}

r_sexp sexp_cdr(r_sexp sexp)
{
    assert(SEXP_PAIR_P(sexp));
    return SEXP_TO_PAIR(sexp)->cdr;
}

r_sexp sexp_set_car_x(r_sexp pair, r_sexp sexp)
{
    assert(SEXP_PAIR_P(pair));
    SEXP_TO_PAIR(pair)->car = sexp;
    return SEXP_UNSPECIFIED;
}

r_sexp sexp_set_cdr_x(r_sexp pair, r_sexp sexp)
{
    assert(SEXP_PAIR_P(pair));
    SEXP_TO_PAIR(pair)->cdr = sexp;
    return SEXP_UNSPECIFIED;
}

static r_sexp sexp_reverse_internal(r_sexp list, r_sexp acc)
{
    return SEXP_NULL_P(list)
           ? acc
           : sexp_reverse_internal(
                   sexp_cdr(list),
                   sexp_cons(sexp_car(list), acc));
}

r_sexp sexp_reverse(r_sexp list)
{
    assert(SEXP_NULL_P(list) || SEXP_PAIR_P(list));
    return sexp_reverse_internal(list, SEXP_NULL);
}

r_sexp sexp_append_x(r_sexp list, r_sexp sexp)
{
    if (SEXP_NULL_P(list))
        return sexp;

    assert(SEXP_PAIR_P(list));

    r_sexp tail = list;
    while (SEXP_PAIR_P(sexp_cdr(tail)))
        tail = sexp_cdr(tail);

    sexp_set_cdr_x(tail, sexp);

    return list;
}

r_sexp sexp_list_p(r_sexp sexp)
{
    if (SEXP_NULL_P(sexp))
        return SEXP_TRUE;
    else if (SEXP_PAIR_P(sexp))
        return sexp_list_p(sexp_cdr(sexp));
    else
        return SEXP_FALSE;
}

r_sexp sexp_list(size_t count, ...)
{
    va_list args;
    va_start(args, count);

    r_sexp res = SEXP_NULL;
    for (size_t i = 0; i < count; ++i)
        res = sexp_cons(va_arg(args, r_sexp), res);

    va_end(args);

    return sexp_reverse(res);
}

size_t sexp_length(r_sexp list)
{
    return SEXP_NULL_P(list)
           ? 0
           : 1 + sexp_length(sexp_cdr(list));
}
