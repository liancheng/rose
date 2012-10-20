#include "detail/state.h"
#include "detail/sexp.h"
#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/writer.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

struct RPair {
    rsexp car;
    rsexp cdr;
};

#define PAIR_TO_SEXP(pair)  (((rsexp) (pair)) | R_PAIR_TAG)
#define PAIR_FROM_SEXP(obj) ((RPair*) ((obj) & (~R_PAIR_TAG)))

typedef void (*ROutputFunction) (rsexp, rsexp);

static rsexp reverse_internal (rsexp list, rsexp acc)
{
    return r_null_p (list)
           ? acc
           : reverse_internal (r_cdr (list),
                               r_cons (r_car (list), acc));
}

static void output_cdr (rsexp           port,
                        rsexp           obj,
                        ROutputFunction output_fn)
{
    if (r_pair_p (obj)) {
        r_port_puts (port, " ");
        output_fn (port, r_car (obj));
        output_cdr (port, r_cdr (obj), output_fn);
    }
    else if (!r_null_p (obj)) {
        r_port_puts (port, " . ");
        output_fn (port, obj);
    }
}

static void output_pair (rsexp           port,
                         rsexp           obj,
                         ROutputFunction output_fn)
{
    assert (r_pair_p (obj));

    r_port_puts (port, "(");
    output_fn (port, r_car (obj));
    output_cdr (port, r_cdr (obj), output_fn);
    r_port_puts (port, ")");
}

static void write_pair (rsexp port, rsexp obj)
{
    output_pair (port, obj, r_write);
}

static void display_pair (rsexp port, rsexp obj)
{
    output_pair (port, obj, r_display);
}

static rsexp vlist (rsize k, va_list args)
{
    rsexp res = R_NULL;

    while (k--)
        res = r_cons (va_arg (args, rsexp), res);

    return r_reverse (res);
}

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
    return PAIR_FROM_SEXP (obj)->car;
}

rsexp r_cdr (rsexp obj)
{
    assert (r_pair_p (obj));
    return PAIR_FROM_SEXP (obj)->cdr;
}

rsexp r_set_car_x (rsexp pair, rsexp obj)
{
    assert (r_pair_p (pair));
    PAIR_FROM_SEXP (pair)->car = obj;
    return R_UNSPECIFIED;
}

rsexp r_set_cdr_x (rsexp pair, rsexp obj)
{
    assert (r_pair_p (pair));
    PAIR_FROM_SEXP (pair)->cdr = obj;
    return R_UNSPECIFIED;
}

rbool r_pair_equal_p (rsexp lhs, rsexp rhs)
{
    return r_pair_p (lhs) &&
           r_pair_p (rhs) &&
           r_equal_p (r_car (lhs), r_car (rhs)) &&
           r_equal_p (r_cdr (lhs), r_cdr (rhs));
}

rsexp r_reverse (rsexp list)
{
    assert (r_null_p (list) || r_pair_p (list));
    return reverse_internal (list, R_NULL);
}

rsexp r_append_x (rsexp list, rsexp obj)
{
    rsexp tail;

    if (r_null_p (list))
        return obj;

    assert (r_pair_p (list));

    for (tail = list; r_pair_p (r_cdr (tail)); )
        tail = r_cdr (tail);

    r_set_cdr_x (tail, obj);

    return list;
}

rbool r_list_p (rsexp obj)
{
    return r_null_p (obj) ||
           (r_pair_p (obj) && r_list_p (r_cdr (obj)));
}

rsexp r_list (rsize k, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, k);
    res = vlist (k, args);
    va_end (args);

    return res;
}

rsize r_length (rsexp list)
{
    rsize n;

    for (n = 0u; !r_null_p (list); list = r_cdr (list), ++n)
        ;

    return n;
}

rsexp r_list_ref (rsexp list, rsize k)
{
    while (k--)
        list = r_cdr (list);

    return r_car (list);
}

void register_pair_type (RState* state)
{
    static RType type = {
        .size    = sizeof (RPair),
        .name    = "pair",
        .write   = write_pair,
        .display = display_pair,
    };

    state->types [R_PAIR_TAG] = &type;
}
