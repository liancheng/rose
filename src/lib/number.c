#include "rose/error.h"
#include "rose/finer_number.h"
#include "rose/pair.h"
#include "rose/primitive.h"
#include "rose/string.h"

static rsexp np_number_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_number_p (r_car (args)));
}

static rsexp np_real_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_real_p (r_car (args)));
}

static rsexp np_integer_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_integer_p (r_car (args)));
}

static rsexp np_exact_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_exact_p (r_car (args)));
}

static rsexp np_inexact_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_inexact_p (r_car (args)));
}

static rsexp np_exact_to_inexact (RState* r, rsexp args)
{
    return r_exact_to_inexact (r, r_car (args));
}

static rsexp np_inexact_to_exact (RState* r, rsexp args)
{
    return r_inexact_to_exact (r, r_car (args));
}

static rsexp np_string_to_number (RState* r, rsexp args)
{
    rsexp num;
    rsexp radix;
    rsexp res;

    r_match_args (r, args, 1, 1, FALSE, &num, &radix);

    if (!r_undefined_p (radix)) {
        r_error_code (r, R_ERR_NOT_IMPLEMENTED,
                      r_string_new (r, "string->number with radix"));
        return R_FAILURE;
    }

    res = r_string_to_number (r, r_car (args));

    return r_failure_p (res) ? R_FALSE : res;
}

RPrimitiveDesc number_primitives [] = {
    { "number?",        np_number_p,         1, 0, FALSE },
    { "complex?",       np_number_p,         1, 0, FALSE },
    { "integer?",       np_integer_p,        1, 0, FALSE },
    { "real?",          np_real_p,           1, 0, FALSE },
    { "rational?",      np_real_p,           1, 0, FALSE },
    { "exact?",         np_exact_p,          1, 0, FALSE },
    { "inexact?",       np_inexact_p,        1, 0, FALSE },
    { "exact->inexact", np_exact_to_inexact, 1, 0, FALSE },
    { "inexact->exact", np_inexact_to_exact, 1, 0, FALSE },
    { "string->number", np_string_to_number, 1, 1, FALSE },
    { NULL }
};
