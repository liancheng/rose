#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/types.h"
#include "rose/write.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

#define PAIR_TO_SEXP(p) (((rsexp)(p) << 2) | R_SEXP_PAIR_TAG)
#define SEXP_TO_PAIR(s) ((RPair*)((s) >> 2))

rsexp r_cons(rsexp car, rsexp cdr)
{
    RPair* pair;

    pair = GC_NEW(RPair);
    pair->car = car;
    pair->cdr = cdr;

    return PAIR_TO_SEXP(pair);
}

rsexp r_car(rsexp sexp)
{
    assert(R_PAIR_P(sexp));
    return SEXP_TO_PAIR(sexp)->car;
}

rsexp r_cdr(rsexp sexp)
{
    assert(R_PAIR_P(sexp));
    return SEXP_TO_PAIR(sexp)->cdr;
}

rsexp r_set_car_x(rsexp pair, rsexp sexp)
{
    assert(R_PAIR_P(pair));
    SEXP_TO_PAIR(pair)->car = sexp;
    return R_SEXP_UNSPECIFIED;
}

rsexp r_set_cdr_x(rsexp pair, rsexp sexp)
{
    assert(R_PAIR_P(pair));
    SEXP_TO_PAIR(pair)->cdr = sexp;
    return R_SEXP_UNSPECIFIED;
}

static rsexp r_reverse_internal(rsexp list, rsexp acc)
{
    return R_NULL_P(list)
           ? acc
           : r_reverse_internal(r_cdr(list),
                                r_cons(r_car(list), acc));
}

rsexp r_reverse(rsexp list)
{
    assert(R_NULL_P(list) || R_PAIR_P(list));
    return r_reverse_internal(list, R_SEXP_NULL);
}

rsexp r_append_x(rsexp list, rsexp sexp)
{
    rsexp tail;

    if (R_NULL_P(list))
        return sexp;

    assert(R_PAIR_P(list));

    for (tail = list; R_PAIR_P(r_cdr(tail)); )
        tail = r_cdr(tail);

    r_set_cdr_x(tail, sexp);

    return list;
}

rboolean r_list_p(rsexp sexp)
{
    return R_NULL_P(sexp) || (R_PAIR_P(sexp) && r_list_p(r_cdr(sexp)));
}

rsexp r_list(rsize count, ...)
{
    va_list args;
    rsize i;

    va_start(args, count);

    rsexp res = R_SEXP_NULL;
    for (i = 0; i < count; ++i)
        res = r_cons(va_arg(args, rsexp), res);

    va_end(args);

    return r_reverse(res);
}

rsize r_length(rsexp list)
{
    return R_NULL_P(list) ? 0 : 1 + r_length(r_cdr(list));
}

static void r_write_pair_cdr(FILE* output, rsexp sexp, RContext* context)
{
    if (R_PAIR_P(sexp)) {
        fprintf(output, " ");
        r_write(output, r_car(sexp), context);
        r_write_pair_cdr(output, r_cdr(sexp), context);
    }
    else if (!R_NULL_P(sexp)) {
        fprintf(output, " . ");
        r_write(output, sexp, context);
    }
}

void r_write_pair(FILE* output, rsexp sexp, RContext* context)
{
    assert(R_PAIR_P(sexp));

    fprintf(output, "(");
    r_write(output, r_car(sexp), context);
    r_write_pair_cdr(output, r_cdr(sexp), context);
    fprintf(output, ")");
}

void r_write_null(FILE* output, rsexp sexp, RContext* context)
{
    fprintf(output, "()");
}

rboolean r_pair_equal_p(rsexp lhs, rsexp rhs)
{
    return R_PAIR_P(lhs) &&
           R_PAIR_P(rhs) &&
           r_equal_p(r_car(lhs), r_car(rhs)) &&
           r_equal_p(r_cdr(lhs), r_cdr(rhs));
}
