#include "detail/state.h"
#include "detail/sexp.h"
#include "rose/eq.h"
#include "rose/memory.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/writer.h"

#include <assert.h>
#include <stdarg.h>

struct RPair {
    rsexp car;
    rsexp cdr;
};

#define PAIR_TO_SEXP(pair)  ((r_cast (rsexp, (pair))) | R_PAIR_TAG)
#define PAIR_FROM_SEXP(obj) (r_cast (RPair*, (obj) & (~R_PAIR_TAG)))

typedef void (*ROutputFunction) (rsexp, rsexp);

static rsexp reverse_internal (RState* state, rsexp list, rsexp acc)
{
    return r_null_p (list)
           ? acc
           : reverse_internal (state,
                               r_cdr (list),
                               r_cons (state, r_car (list), acc));
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

static rsexp vlist (RState* state, rsize k, va_list args)
{
    rsexp res = R_NULL;

    while (k--)
        res = r_cons (state, va_arg (args, rsexp), res);

    return r_reverse (state, res);
}

static rbool pair_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    return r_pair_p (lhs) &&
           r_pair_p (rhs) &&
           r_equal_p (state, r_car (lhs), r_car (rhs)) &&
           r_equal_p (state, r_cdr (lhs), r_cdr (rhs));
}

rsexp r_cons (RState* state, rsexp car, rsexp cdr)
{
    RPair* pair;

    pair = R_NEW (state, RPair);
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

rsexp r_reverse (RState* state, rsexp list)
{
    assert (r_null_p (list) || r_pair_p (list));
    return reverse_internal (state, list, R_NULL);
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
    rbool res;

    while (TRUE) {
        if (r_null_p (obj)) {
            res = TRUE;
            break;
        }

        if (!r_pair_p (obj)) {
            res = FALSE;
            break;
        }

        obj = r_cdr (obj);
    }

    return res;
}

rsexp r_list (RState* state, rsize k, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, k);
    res = vlist (state, k, args);
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

RTypeInfo* init_pair_type_info (RState* state)
{
    RTypeInfo* type = R_NEW0 (state, RTypeInfo);

    type->size         = sizeof (RPair);
    type->name         = "pair";
    type->ops.write    = write_pair;
    type->ops.display  = display_pair;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = pair_equal_p;
    type->ops.mark     = NULL;
    type->ops.destruct = NULL;

    return type;
}
