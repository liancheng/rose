#include "rose/eq.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/math.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/primitive.h"

static rsexp np_add (RState* r, rsexp args)
{
    return r_fold (r, r_add, R_ZERO, args);
}

static rsexp np_minus (RState* r, rsexp args)
{
    rsexp diff;

    r_gc_scope_open (r);

    if (r_null_p (r_cdr (args))) {
        diff = r_negate (r, r_car (args));
        goto exit;
    }

    diff = r_car (args);
    args = r_cdr (args);

    while (!r_null_p (args)) {
        ensure_or_goto (diff = r_minus (r, diff, r_car (args)), exit);
        args = r_cdr (args);
    }

exit:
    r_gc_scope_close_and_protect (r, diff);

    return diff;
}

static rsexp np_multiply (RState* r, rsexp args)
{
    return r_fold (r, r_multiply, R_ONE, args);
}

static rsexp np_divide (RState* r, rsexp args)
{
    rsexp quot;

    r_gc_scope_open (r);

    if (r_null_p (r_cdr (args))) {
        quot = r_invert (r, r_car (args));
        goto exit;
    }

    quot = r_car (args);
    args = r_cdr (args);

    while (!r_null_p (args)) {
        ensure_or_goto (quot = r_divide (r, quot, r_car (args)), exit);
        args = r_cdr (args);
    }

exit:
    r_gc_scope_close_and_protect (r, quot);

    return quot;
}

static rsexp np_num_cmp_loop (RState* r, rsexp args,
                              rsexp (*cmp) (RState*, rsexp, rsexp))
{
    rsexp car;
    rsexp res;

    if (r_null_p (args))
        return R_TRUE;

    car = r_car (args);
    args = r_cdr (args);

    while (!r_null_p (args)) {
        res = cmp (r, car, r_car (args));

        if (res != R_TRUE)
            return res;

        car = r_car (args);
        args = r_cdr (args);
    }

    return res;
}

static rsexp np_num_eq_p (RState* r, rsexp args)
{
    return np_num_cmp_loop (r, args, r_num_eq_p);
}

static rsexp np_num_lt_p (RState* r, rsexp args)
{
    return np_num_cmp_loop (r, args, r_num_lt_p);
}

static rsexp np_num_le_p (RState* r, rsexp args)
{
    return np_num_cmp_loop (r, args, r_num_le_p);
}

static rsexp np_num_gt_p (RState* r, rsexp args)
{
    return np_num_cmp_loop (r, args, r_num_gt_p);
}

static rsexp np_num_ge_p (RState* r, rsexp args)
{
    return np_num_cmp_loop (r, args, r_num_ge_p);
}

const RPrimitiveDesc math_primitives [] = {
    { "+",  np_add,      0, 0, TRUE },
    { "*",  np_multiply, 0, 0, TRUE },
    { "-",  np_minus,    1, 0, TRUE },
    { "/",  np_divide,   1, 0, TRUE },
    { "=",  np_num_eq_p, 0, 0, TRUE },
    { "<",  np_num_lt_p, 0, 0, TRUE },
    { "<=", np_num_le_p, 0, 0, TRUE },
    { ">",  np_num_gt_p, 0, 0, TRUE },
    { ">=", np_num_ge_p, 0, 0, TRUE },
    { NULL }
};
