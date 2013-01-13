#include "detail/env.h"
#include "detail/state.h"
#include "detail/sexp.h"
#include "rose/bytevector.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/procedure.h"
#include "rose/primitive.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <assert.h>
#include <stdarg.h>

typedef struct RPair RPair;

struct RPair {
    R_OBJECT_HEADER
    rsexp car;
    rsexp cdr;
};

#define pair_from_sexp(obj)     (r_cast (RPair*, (obj)))
#define pair_to_sexp(pair)      (r_cast (rsexp, (pair)))

typedef rsexp (*ROutputFunc) (RState* r, rsexp, rsexp);

static rsexp output_cdr (RState* r,
                         rsexp port,
                         rsexp obj,
                         ROutputFunc output_fn)
{
    if (r_pair_p (obj)) {
        r_port_write_char (r, port, ' ');
        ensure (output_fn (r, port, r_car (obj)));
        ensure (output_cdr (r, port, r_cdr (obj), output_fn));
    }
    else if (!r_null_p (obj)) {
        ensure (r_port_puts (r, port, " . "));
        ensure (output_fn (r, port, obj));
    }

    return R_UNSPECIFIED;
}

static rsexp pair_output (RState* r,
                          rsexp port,
                          rsexp obj,
                          ROutputFunc output_fn)
{
    ensure (r_port_puts (r, port, "("));
    ensure (output_fn (r, port, r_car (obj)));
    ensure (output_cdr (r, port, r_cdr (obj), output_fn));
    ensure (r_port_puts (r, port, ")"));

    return R_UNSPECIFIED;
}

static rsexp pair_write (RState* r, rsexp port, rsexp obj)
{
    return pair_output (r, port, obj, r_port_write);
}

static rsexp pair_display (RState* r, rsexp port, rsexp obj)
{
    return pair_output (r, port, obj, r_port_display);
}

static void pair_mark (RState* r, rsexp obj)
{
    assert (r_pair_p (obj));
    r_gc_mark (r, pair_from_sexp (obj)->car);
    r_gc_mark (r, pair_from_sexp (obj)->cdr);
}

static rbool pair_equal_p (RState* r, rsexp lhs, rsexp rhs)
{
    return r_pair_p (lhs) &&
           r_pair_p (rhs) &&
           r_equal_p (r, r_car (lhs), r_car (rhs)) &&
           r_equal_p (r, r_cdr (lhs), r_cdr (rhs));
}

rbool r_pair_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_PAIR;
}

rsexp r_cons (RState* r, rsexp car, rsexp cdr)
{
    RPair* pair = r_object_new (r, RPair, R_TAG_PAIR);

    if (!pair)
        return R_FAILURE;

    pair->car = car;
    pair->cdr = cdr;

    return pair_to_sexp (pair);
}

rsexp r_car (rsexp obj)
{
    assert (r_pair_p (obj));
    return pair_from_sexp (obj)->car;
}

rsexp r_cdr (rsexp obj)
{
    assert (r_pair_p (obj));
    return pair_from_sexp (obj)->cdr;
}

void r_set_car_x (rsexp pair, rsexp obj)
{
    assert (r_pair_p (obj));
    pair_from_sexp (pair)->car = obj;
}

void r_set_cdr_x (rsexp pair, rsexp obj)
{
    assert (r_pair_p (pair));
    pair_from_sexp (pair)->cdr = obj;
}

rsexp r_reverse (RState* r, rsexp list)
{
    rsexp res = R_NULL;

    while (!r_null_p (list)) {
        if (!r_pair_p (list)) {
            r_error_code (r, R_ERR_WRONG_TYPE_ARG, list);
            res = R_FAILURE;
            goto exit;
        }

        ensure_or_goto (res = r_cons (r, r_car (list), res), exit);

        list = r_cdr (list);
    }

exit:
    return res;
}

rsexp r_reverse_x (RState* r, rsexp list)
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
            r_error_code (r, R_ERR_WRONG_TYPE_ARG, list);
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

rsexp r_append_x (RState* r, rsexp list, rsexp obj)
{
    rsexp tail;

    if (r_null_p (list))
        return obj;

    if (!r_pair_p (list)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, list);
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

rsexp r_vlist (RState* r, rsize k, va_list args)
{
    rsize i;
    rsexp res = R_NULL;

    r_gc_scope_open (r);

    for (i = 0; i < k; ++i)
        ensure_or_goto (res = r_cons (r, va_arg (args, rsexp), res), exit);

    res = r_reverse_x (r, res);

exit:
    r_gc_scope_close_and_protect (r, res);

    return res;
}

rsexp r_list (RState* r, rsize k, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, k);
    res = r_vlist (r, k, args);
    va_end (args);

    return res;
}

rsexp r_length (RState* r, rsexp list)
{
    rsize n;

    for (n = 0u; !r_null_p (list); list = r_cdr (list), ++n)
        if (!r_pair_p (list)) {
            r_error_code (r, R_ERR_WRONG_TYPE_ARG, list);
            return R_FAILURE;
        }

    return r_uint_to_sexp (n);
}

rsexp r_list_ref (RState* r, rsexp seq, rsize k)
{
    rsize i;
    rsexp res;

    for (i = 0; i <= k; ++i) {
        if (r_null_p (seq))
            break;

        if (!r_pair_p (seq)) {
            r_error_code (r, R_ERR_WRONG_TYPE_ARG, seq);
            return R_FAILURE;
        }

        res = r_car (seq);
        seq = r_cdr (seq);
    }

    if (i <= k) {
        r_error_code (r, R_ERR_INDEX_OVERFLOW);
        return R_FAILURE;
    }

    return res;
}

rsexp np_cons (RState* r, rsexp args)
{
    rsexp car, cdr;
    r_match_args (r, args, 2, 0, FALSE, &car, &cdr);
    return r_cons (r, car, cdr);
}

rsexp np_pair_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_pair_p (r_car (args)));
}

rsexp np_null_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_null_p (r_car (args)));
}

rsexp np_car (RState* r, rsexp args)
{
    rsexp pair = r_car (args);

    if (!r_pair_p (pair)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    return r_car (pair);
}

rsexp np_cdr (RState* r, rsexp args)
{
    rsexp pair = r_car (args);

    if (!r_pair_p (pair)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    return r_cdr (pair);
}

rsexp np_set_car_x (RState* r, rsexp args)
{
    rsexp pair = r_car (args);
    rsexp car = r_cadr (args);

    if (!r_pair_p (pair)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    r_set_car_x (pair, car);

    return R_UNSPECIFIED;
}

rsexp np_set_cdr_x (RState* r, rsexp args)
{
    rsexp pair = r_car (args);
    rsexp cdr = r_cadr (args);

    if (!r_pair_p (pair)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    r_set_cdr_x (pair, cdr);

    return R_UNDEFINED;
}

rsexp np_list (RState* r, rsexp args)
{
    return args;
}

rsexp np_reverse (RState* r, rsexp args)
{
    return r_reverse (r, r_car (args));
}

rsexp np_reverse_x (RState* r, rsexp args)
{
    return r_reverse_x (r, r_car (args));
}

rsexp np_append_x (RState* r, rsexp args)
{
    return r_append_x (r, r_car (args), r_cadr (args));
}

rsexp np_list_ref (RState* r, rsexp args)
{
    rsexp list, k;

    list = r_car (args);
    k = r_cadr (args);

    if (!r_small_int_p (k)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, k);
        return R_FAILURE;
    }

    return r_list_ref (r, list, r_uint_from_sexp (k));
}

void pair_init_primitives (RState* r, rsexp* env)
{
    bind_primitive_x (r, env, "cons",     np_cons,      2, 0, FALSE);
    bind_primitive_x (r, env, "pair?",    np_pair_p,    1, 0, FALSE);
    bind_primitive_x (r, env, "null?",    np_null_p,    1, 0, FALSE);
    bind_primitive_x (r, env, "car",      np_car,       1, 0, FALSE);
    bind_primitive_x (r, env, "cdr",      np_cdr,       1, 0, FALSE);
    bind_primitive_x (r, env, "set-car!", np_set_car_x, 2, 0, FALSE);
    bind_primitive_x (r, env, "set-cdr!", np_set_cdr_x, 2, 0, FALSE);
    bind_primitive_x (r, env, "list",     np_list,      0, 0, TRUE);
    bind_primitive_x (r, env, "reverse",  np_reverse,   1, 0, FALSE);
    bind_primitive_x (r, env, "reverse!", np_reverse_x, 1, 0, FALSE);
    bind_primitive_x (r, env, "append!",  np_append_x,  2, 0, FALSE);
    bind_primitive_x (r, env, "list-ref", np_list_ref,  2, 0, FALSE);
}

RTypeInfo pair_type = {
    .size = sizeof (RPair),
    .name = "pair",
    .ops = {
        .write = pair_write,
        .display = pair_display,
        .eqv_p = NULL,
        .equal_p = pair_equal_p,
        .mark = pair_mark,
        .finalize = NULL
    }
};
