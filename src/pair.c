#include "rose/pair.h"

#include <assert.h>
#include <gc.h>
#include <stdlib.h>

static r_pair* pair_new()
{
    r_pair* pair = (r_pair*)GC_malloc(sizeof(r_pair));
    pair->car = SEXP_UNSPECIFIED;
    pair->cdr = SEXP_UNSPECIFIED;
    return pair;
}

r_sexp sexp_cons(r_sexp car, r_sexp cdr)
{
    r_pair* pair = pair_new();
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
    return SEXP_NULL_P(list) ?
        acc :
        sexp_reverse_internal(sexp_cdr(list),
                              sexp_cons(sexp_car(list), acc));
}

r_sexp sexp_reverse(r_sexp list)
{
    return sexp_reverse_internal(list, SEXP_NULL);
}

r_sexp sexp_append_x(r_sexp list, r_sexp sexp)
{
    if (SEXP_NULL_P(list))
        return sexp;

    assert(SEXP_PAIR_P(list));

    r_sexp tail = list;
    for (; SEXP_PAIR_P(sexp_cdr(tail)); tail = sexp_cdr(tail))
        ;

    sexp_set_cdr_x(tail, sexp);

    return list;
}
