#include "rose/error.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/primitive.h"
#include "rose/string.h"

static rsexp np_string_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_string_p (r_car (args)));
}

static rsexp np_make_string (RState* r, rsexp args)
{
    rsexp size, ch;

    r_match_args (r, args, 1, 1, FALSE, &size, &ch);

    if (!r_small_int_p (size)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, size);
        return R_FAILURE;
    }

    if (r_undefined_p (ch))
        ch = r_char_to_sexp ('\0');

    if (!r_char_p (ch)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, ch);
        return R_FAILURE;
    }

    return r_make_string (r, r_uint_from_sexp (size), r_char_from_sexp (ch));
}

static rsexp np_string_length (RState* r, rsexp args)
{
    rsexp obj = r_car (args);

    if (!r_string_p (obj)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, obj);
        return R_FAILURE;
    }

    return r_string_length (obj);
}

static rsexp np_string_ref (RState* r, rsexp args)
{
    rsexp obj = r_car (args);
    rsexp k = r_cadr (args);

    if (!r_string_p (obj)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, obj);
        return R_FAILURE;
    }

    if (!r_small_int_p (k)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, k);
        return R_FAILURE;
    }

    return r_string_ref (r, obj, r_uint_from_sexp (k));
}

static rsexp np_string_set_x (RState* r, rsexp args)
{
    rsexp obj = r_car (args);
    rsexp k = r_cadr (args);
    rsexp ch = r_caddr (args);

    if (!r_string_p (obj)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, obj);
        return R_FAILURE;
    }

    if (!r_small_int_p (k)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, k);
        return R_FAILURE;
    }

    if (!r_char_p (ch)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, ch);
        return R_FAILURE;
    }

    return r_string_set_x (r, obj, r_uint_from_sexp (k), r_char_from_sexp (ch));
}

const RPrimitiveDesc string_primitives [] = {
    { "string?",       np_string_p,       1, 0, FALSE },
    { "make-string",   np_make_string,    1, 1, FALSE },
    { "string-length", np_string_length,  1, 0, FALSE },
    { "string-ref",    np_string_ref,     2, 0, FALSE },
    { "string-set!",   np_string_set_x,   3, 0, FALSE },
    { NULL }
};
