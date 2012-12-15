#include "detail/error.h"
#include "detail/state.h"
#include "detail/sexp.h"
#include "rose/eq.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/string.h"

#include <assert.h>
#include <stdarg.h>

typedef struct RPair RPair;

struct RPair {
    rsexp car;
    rsexp cdr;
};

#define pair_to_sexp(pair)  ((r_cast (rsexp, (pair))) | R_PAIR_TAG)
#define pair_from_sexp(obj) (r_cast (RPair*, (obj) & (~R_PAIR_TAG)))

typedef rsexp (*ROutputFunc) (RState* state, rsexp, rsexp);

static rsexp output_cdr (RState*     state,
                         rsexp       port,
                         rsexp       obj,
                         ROutputFunc output_fn)
{
    if (r_pair_p (obj)) {
        r_port_write_char (state, port, ' ');
        ensure (output_fn (state, port, r_car (obj)));
        ensure (output_cdr (state, port, r_cdr (obj), output_fn));
    }
    else if (!r_null_p (obj)) {
        ensure (r_port_puts (state, port, " . "));
        ensure (output_fn (state, port, obj));
    }

    return R_UNSPECIFIED;
}

static rsexp output_pair (RState*     state,
                          rsexp       port,
                          rsexp       obj,
                          ROutputFunc output_fn)
{
    ensure (r_port_puts (state, port, "("));
    ensure (output_fn (state, port, r_car (obj)));
    ensure (output_cdr (state, port, r_cdr (obj), output_fn));
    ensure (r_port_puts (state, port, ")"));

    return R_UNSPECIFIED;
}

static rsexp write_pair (RState* state, rsexp port, rsexp obj)
{
    return output_pair (state, port, obj, r_port_write);
}

static rsexp display_pair (RState* state, rsexp port, rsexp obj)
{
    return output_pair (state, port, obj, r_port_display);
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

    pair = r_new (state, RPair);
    pair->car = car;
    pair->cdr = cdr;

    return pair_to_sexp (pair);
}

rsexp r_car (rsexp obj)
{
    return pair_from_sexp (obj)->car;
}

rsexp r_cdr (rsexp obj)
{
    return pair_from_sexp (obj)->cdr;
}

void r_set_car_x (rsexp pair, rsexp obj)
{
    pair_from_sexp (pair)->car = obj;
}

void r_set_cdr_x (rsexp pair, rsexp obj)
{
    pair_from_sexp (pair)->cdr = obj;
}

rsexp r_reverse (RState* state, rsexp list)
{
    rsexp node;
    rsexp next;
    rsexp prev;
    rsexp res;

    node = list;
    prev = R_NULL;

    while (!r_null_p (node)) {
        if (!r_pair_p (node)) {
            res = R_FAILURE;
            error_wrong_type_arg (state, "pair", list);
            goto exit;
        }

        next = r_cdr (node);
        r_set_cdr_x (node, prev);
        prev = node;
        node = next;
    }

    res = prev;

exit:
    return res;
}

rsexp r_append_x (RState* state, rsexp list, rsexp obj)
{
    rsexp tail;

    if (r_null_p (list))
        return obj;

    if (!r_pair_p (list)) {
        error_wrong_type_arg (state, "pair", list);
        return R_FAILURE;
    }

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

rsexp r_vlist (RState* state, rsize k, va_list args)
{
    rsize i;
    rsexp res = R_NULL;

    for (i = 0; i < k; ++i)
        res = r_cons (state, va_arg (args, rsexp), res);

    return r_reverse (state, res);
}

rsexp r_list (RState* state, rsize k, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, k);
    res = r_vlist (state, k, args);
    va_end (args);

    return res;
}

rsexp r_length (RState* state, rsexp list)
{
    rsize n;

    for (n = 0u; !r_null_p (list); list = r_cdr (list), ++n)
        if (!r_pair_p (list)) {
            error_wrong_type_arg (state, "pair", list);
            return R_FAILURE;
        }

    return r_uint_to_sexp (n);
}

rsexp r_list_ref (rsexp list, rsize k)
{
    while (k--)
        list = r_cdr (list);

    return r_car (list);
}

void init_pair_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RPair);
    type->name         = "pair";
    type->ops.write    = write_pair;
    type->ops.display  = display_pair;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = pair_equal_p;
    type->ops.mark     = NULL;
    type->ops.destruct = NULL;

    state->builtin_types [R_PAIR_TAG] = type;
}
