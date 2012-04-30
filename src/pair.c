#include "rose/pair.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdlib.h>
#include <stdarg.h>

#define SEXP_FROM_PAIR(p) (((rsexp)(p) << 2) | SEXP_PAIR_TAG)
#define SEXP_TO_PAIR(s)   ((RPair*)((s) >> 2))

rsexp sexp_cons(rsexp car, rsexp cdr)
{
    RPair* pair = GC_NEW(RPair);
    pair->car = car;
    pair->cdr = cdr;
    return SEXP_FROM_PAIR(pair);
}

rsexp sexp_car(rsexp sexp)
{
    assert(SEXP_PAIR_P(sexp));
    return SEXP_TO_PAIR(sexp)->car;
}

rsexp sexp_cdr(rsexp sexp)
{
    assert(SEXP_PAIR_P(sexp));
    return SEXP_TO_PAIR(sexp)->cdr;
}

rsexp sexp_set_car_x(rsexp pair, rsexp sexp)
{
    assert(SEXP_PAIR_P(pair));
    SEXP_TO_PAIR(pair)->car = sexp;
    return SEXP_UNSPECIFIED;
}

rsexp sexp_set_cdr_x(rsexp pair, rsexp sexp)
{
    assert(SEXP_PAIR_P(pair));
    SEXP_TO_PAIR(pair)->cdr = sexp;
    return SEXP_UNSPECIFIED;
}

static rsexp sexp_reverse_internal(rsexp list, rsexp acc)
{
    return SEXP_NULL_P(list)
           ? acc
           : sexp_reverse_internal(
                   sexp_cdr(list),
                   sexp_cons(sexp_car(list), acc));
}

rsexp sexp_reverse(rsexp list)
{
    assert(SEXP_NULL_P(list) || SEXP_PAIR_P(list));
    return sexp_reverse_internal(list, SEXP_NULL);
}

rsexp sexp_append_x(rsexp list, rsexp sexp)
{
    if (SEXP_NULL_P(list))
        return sexp;

    assert(SEXP_PAIR_P(list));

    rsexp tail = list;
    while (SEXP_PAIR_P(sexp_cdr(tail)))
        tail = sexp_cdr(tail);

    sexp_set_cdr_x(tail, sexp);

    return list;
}

rsexp sexp_list_p(rsexp sexp)
{
    if (SEXP_NULL_P(sexp))
        return SEXP_TRUE;
    else if (SEXP_PAIR_P(sexp))
        return sexp_list_p(sexp_cdr(sexp));
    else
        return SEXP_FALSE;
}

rsexp sexp_list(size_t count, ...)
{
    va_list args;
    va_start(args, count);

    rsexp res = SEXP_NULL;
    for (size_t i = 0; i < count; ++i)
        res = sexp_cons(va_arg(args, rsexp), res);

    va_end(args);

    return sexp_reverse(res);
}

size_t sexp_length(rsexp list)
{
    return SEXP_NULL_P(list)
           ? 0
           : 1 + sexp_length(sexp_cdr(list));
}
