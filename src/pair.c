#include "rose/pair.h"

#include <assert.h>
#include <gc.h>
#include <stdlib.h>

r_sexp sexp_new()
{
    return (r_sexp)GC_malloc(sizeof(struct r_sexp_struct));
}

void sexp_free(r_sexp sexp)
{
    GC_free(sexp);
}

r_sexp sexp_cons(r_sexp car, r_sexp cdr)
{
    r_sexp sexp = sexp_new();
    sexp->type = SEXP_PAIR;
    sexp->value.pair.car = car;
    sexp->value.pair.cdr = cdr;
    return sexp;
}

r_sexp sexp_car(r_sexp sexp)
{
    assert(sexp_pair_p(sexp));
    return sexp->value.pair.car;
}

r_sexp sexp_cdr(r_sexp sexp)
{
    assert(sexp_pair_p(sexp));
    return sexp->value.pair.cdr;
}

r_sexp sexp_set_car_x(r_sexp pair, r_sexp sexp)
{
    assert(sexp_pair_p(pair));
    pair->value.pair.car = sexp;
    return SEXP_UNDEFINED;
}

r_sexp sexp_set_cdr_x(r_sexp pair, r_sexp sexp)
{
    assert(sexp_pair_p(pair));
    pair->value.pair.cdr = sexp;
    return SEXP_UNDEFINED;
}

static r_sexp sexp_reverse_internal(r_sexp list, r_sexp acc)
{
    return sexp_null_p(list) ?
        acc :
        sexp_reverse_internal(sexp_cdr(list),
                              sexp_cons(sexp_car(list), acc));
}

r_sexp sexp_reverse(r_sexp list)
{
    return sexp_reverse_internal(list, SEXP_NULL);
}

r_sexp sexp_append(r_sexp list, r_sexp sexp)
{
    if (sexp_null_p(list))
        return sexp;

    assert(sexp_pair_p(list));

    r_sexp tail = list;
    for (; sexp_pair_p(sexp_cdr(tail)); tail = sexp_cdr(tail))
        ;

    sexp_set_cdr_x(tail, sexp);

    return list;
}
