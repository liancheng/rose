#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/symbol.h"
#include "rose/write.h"

#include <assert.h>
#include <stdarg.h>

#define PAIR_TO_SEXP(ptr) (((rsexp) (ptr)) | R_SEXP_PAIR_TAG)
#define SEXP_TO_PAIR(obj) ((RPair*) ((obj) & (~R_SEXP_PAIR_TAG)))

rsexp r_cons (rsexp car, rsexp cdr)
{
    RPair* pair;

    pair = GC_NEW (RPair);
    pair->car = car;
    pair->cdr = cdr;

    return PAIR_TO_SEXP (pair);
}

rsexp r_car (rsexp obj)
{
    assert (r_pair_p (obj));
    return SEXP_TO_PAIR (obj)->car;
}

rsexp r_cdr (rsexp obj)
{
    assert (r_pair_p (obj));
    return SEXP_TO_PAIR (obj)->cdr;
}

rsexp r_set_car (rsexp pair, rsexp obj)
{
    assert (r_pair_p (pair));
    SEXP_TO_PAIR (pair)->car = obj;
    return R_SEXP_UNSPECIFIED;
}

rsexp r_set_cdr (rsexp pair, rsexp obj)
{
    assert (r_pair_p (pair));
    SEXP_TO_PAIR (pair)->cdr = obj;
    return R_SEXP_UNSPECIFIED;
}

static inline rsexp r_reverse_internal (rsexp list, rsexp acc)
{
    return r_null_p (list)
           ? acc
           : r_reverse_internal (r_cdr (list),
                                r_cons (r_car (list), acc));
}

rsexp r_reverse (rsexp list)
{
    assert (r_null_p (list) || r_pair_p (list));
    return r_reverse_internal (list, R_SEXP_NULL);
}

rsexp r_append_x (rsexp list, rsexp obj)
{
    rsexp tail;

    if (r_null_p (list))
        return obj;

    assert (r_pair_p (list));

    for (tail = list; r_pair_p (r_cdr (tail)); )
        tail = r_cdr (tail);

    r_set_cdr (tail, obj);

    return list;
}

rboolean r_list_p (rsexp obj)
{
    return r_null_p (obj) ||
           (r_pair_p (obj) && r_list_p (r_cdr (obj)));
}

rsexp r_list (rsize count, ...)
{
    va_list args;
    rsize i;

    va_start (args, count);

    rsexp res = R_SEXP_NULL;
    for (i = 0; i < count; ++i)
        res = r_cons (va_arg (args, rsexp), res);

    va_end (args);

    return r_reverse (res);
}

rsize r_length (rsexp list)
{
    return r_null_p (list) ? 0 : 1 + r_length (r_cdr (list));
}

rboolean r_pair_equal_p (rsexp lhs, rsexp rhs)
{
    return r_pair_p (lhs) &&
           r_pair_p (rhs) &&
           r_equal_p (r_car (lhs), r_car (rhs)) &&
           r_equal_p (r_cdr (lhs), r_cdr (rhs));
}

typedef void (*ROutputFunction) (rsexp, rsexp, rsexp);

static void output_cdr (rsexp           port,
                        rsexp           obj,
                        ROutputFunction output_fn,
                        rsexp           context)
{
    if (r_pair_p (obj)) {
        r_port_puts (port, " ");
        output_fn (port, r_car (obj), context);
        output_cdr (port, r_cdr (obj), output_fn, context);
    }
    else if (!r_null_p (obj)) {
        r_port_puts (port, " . ");
        output_fn (port, obj, context);
    }
}

static void output_pair (rsexp           port,
                         rsexp           obj,
                         ROutputFunction output_fn,
                         rsexp           context)
{
    assert (r_pair_p (obj));

    r_port_puts (port, "(");
    output_fn (port, r_car (obj), context);
    output_cdr (port, r_cdr (obj), output_fn, context);
    r_port_puts (port, ")");
}

void r_write_pair (rsexp port, rsexp obj, rsexp context)
{
    output_pair (port, obj, r_write, context);
}

void r_write_null (rsexp port, rsexp obj, rsexp context)
{
    r_port_puts (port, "()");
}

void r_display_pair (rsexp port, rsexp obj, rsexp context)
{
    output_pair (port, obj, r_display, context);
}

void r_display_null (rsexp port, rsexp obj, rsexp context)
{
    r_port_puts (port, "()");
}
