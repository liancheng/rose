#include "rose/error.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/primitive.h"
#include "rose/string.h"

static rsexp np_string_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_string_p (r_car (args)));
}

static rsexp np_string_length (RState* r, rsexp args)
{
    rsexp obj = r_car (args);

    if (!r_string_p (obj)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, r_car (obj));
        return R_FAILURE;
    }

    return r_uint_to_sexp (r_string_length (obj));
}

const RPrimitiveDesc string_primitives [] = {
    { "string?",       np_string_p,      1, 0, FALSE },
    { "string-length", np_string_length, 1, 0, FALSE },
    { NULL }
};
