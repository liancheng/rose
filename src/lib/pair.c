#include "rose/error.h"
#include "rose/finer_number.h"
#include "rose/pair.h"
#include "rose/primitive.h"

static rsexp np_cons (RState* r, rsexp args)
{
    rsexp car, cdr;
    r_match_args (r, args, 2, 0, FALSE, &car, &cdr);
    return r_cons (r, car, cdr);
}

static rsexp np_pair_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_pair_p (r_car (args)));
}

static rsexp np_null_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_null_p (r_car (args)));
}

static rsexp np_car (RState* r, rsexp args)
{
    rsexp pair = r_car (args);

    if (!r_pair_p (pair)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    return r_car (pair);
}

static rsexp np_cdr (RState* r, rsexp args)
{
    rsexp pair = r_car (args);

    if (!r_pair_p (pair)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    return r_cdr (pair);
}

static rsexp np_set_car_x (RState* r, rsexp args)
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

static rsexp np_set_cdr_x (RState* r, rsexp args)
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

static rsexp np_list (RState* r, rsexp args)
{
    return args;
}

static rsexp np_reverse (RState* r, rsexp args)
{
    return r_reverse (r, r_car (args));
}

static rsexp np_reverse_x (RState* r, rsexp args)
{
    return r_reverse_x (r, r_car (args));
}

static rsexp np_append_x (RState* r, rsexp args)
{
    return r_append_x (r, r_car (args), r_cadr (args));
}

static rsexp np_list_ref (RState* r, rsexp args)
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

RPrimitiveDesc pair_primitives [] = {
    { "cons",     np_cons,      2, 0, FALSE },
    { "pair?",    np_pair_p,    1, 0, FALSE },
    { "null?",    np_null_p,    1, 0, FALSE },
    { "car",      np_car,       1, 0, FALSE },
    { "cdr",      np_cdr,       1, 0, FALSE },
    { "set-car!", np_set_car_x, 2, 0, FALSE },
    { "set-cdr!", np_set_cdr_x, 2, 0, FALSE },
    { "list",     np_list,      0, 0, TRUE },
    { "reverse",  np_reverse,   1, 0, FALSE },
    { "reverse!", np_reverse_x, 1, 0, FALSE },
    { "append!",  np_append_x,  2, 0, FALSE },
    { "list-ref", np_list_ref,  2, 0, FALSE },
    { NULL }
};
