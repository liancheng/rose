#include "rose/pair.h"
#include "rose/types.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

#define SEXP_FROM_PAIR(p) (((rsexp)(p) << 2) | SEXP_PAIR_TAG)
#define SEXP_TO_PAIR(s)   ((RPair*)((s) >> 2))

rsexp r_cons(rsexp car, rsexp cdr)
{
    RPair* pair;

    pair      = GC_NEW(RPair);
    pair->car = car;
    pair->cdr = cdr;

    return SEXP_FROM_PAIR(pair);
}

rsexp r_car(rsexp sexp)
{
    assert(SEXP_PAIR_P(sexp));
    return SEXP_TO_PAIR(sexp)->car;
}

rsexp r_cdr(rsexp sexp)
{
    assert(SEXP_PAIR_P(sexp));
    return SEXP_TO_PAIR(sexp)->cdr;
}

rsexp r_set_car_x(rsexp pair, rsexp sexp)
{
    assert(SEXP_PAIR_P(pair));
    SEXP_TO_PAIR(pair)->car = sexp;
    return SEXP_UNSPECIFIED;
}

rsexp r_set_cdr_x(rsexp pair, rsexp sexp)
{
    assert(SEXP_PAIR_P(pair));
    SEXP_TO_PAIR(pair)->cdr = sexp;
    return SEXP_UNSPECIFIED;
}

static rsexp r_reverse_internal(rsexp list, rsexp acc)
{
    return SEXP_NULL_P(list)
           ? acc
           : r_reverse_internal(r_cdr(list),
                                r_cons(r_car(list), acc));
}

rsexp r_reverse(rsexp list)
{
    assert(SEXP_NULL_P(list) || SEXP_PAIR_P(list));
    return r_reverse_internal(list, SEXP_NULL);
}

rsexp r_append_x(rsexp list, rsexp sexp)
{
    rsexp tail;

    if (SEXP_NULL_P(list))
        return sexp;

    assert(SEXP_PAIR_P(list));

    for (tail = list; SEXP_PAIR_P(r_cdr(tail)); )
        tail = r_cdr(tail);

    r_set_cdr_x(tail, sexp);

    return list;
}

rboolean r_list_p(rsexp sexp)
{
    return SEXP_NULL_P(sexp) || (SEXP_PAIR_P(sexp) && r_list_p(r_cdr(sexp)));
}

rsexp r_list(rsize count, ...)
{
    va_list args;
    rsize i;

    va_start(args, count);

    rsexp res = SEXP_NULL;
    for (i = 0; i < count; ++i)
        res = r_cons(va_arg(args, rsexp), res);

    va_end(args);

    return r_reverse(res);
}

rsize r_length(rsexp list)
{
    return SEXP_NULL_P(list) ? 0 : 1 + r_length(r_cdr(list));
}
