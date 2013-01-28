#include "rose/bytevector.h"
#include "rose/error.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/primitive.h"

static rsexp np_make_bytevector (RState* r, rsexp args)
{
    rsexp k, fill;

    r_match_args (r, args, 1, 1, FALSE, &k, &fill);

    if (r_undefined_p (fill))
        fill = R_UNSPECIFIED;

    if (!r_small_int_p (k)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, k);
        return R_FAILURE;
    }

    if (!r_byte_p (fill)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, fill);
        return R_FAILURE;
    }

    return r_bytevector_new (r, r_uint_from_sexp (k), r_uint_from_sexp (fill));
}

static rsexp np_bytevector_length (RState* r, rsexp args)
{
    rsexp obj = r_car (args);

    if (!r_bytevector_p (obj)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, obj);
        return R_FAILURE;
    }

    return r_bytevector_length (obj);
}

static rsexp np_list_to_bytevector (RState* r, rsexp args)
{
    return r_list_to_bytevector (r, r_car (args));
}

static rsexp np_bytevector_u8_ref (RState* r, rsexp args)
{
    rsexp obj, k;

    r_match_args (r, args, 2, 0, FALSE, &obj, &k);

    if (!r_bytevector_p (obj)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, obj);
        return R_FAILURE;
    }

    if (!r_small_int_p (k)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, k);
        return R_FAILURE;
    }

    return r_bytevector_u8_ref (r, obj, r_uint_from_sexp (k));
}

static rsexp np_bytevector_u8_set_x (RState* r, rsexp args)
{
    rsexp obj, k, byte;

    r_match_args (r, args, 3, 0, FALSE, &obj, &k, &byte);

    if (!r_bytevector_p (obj)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, obj);
        return R_FAILURE;
    }

    if (!r_small_int_p (k)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, k);
        return R_FAILURE;
    }

    if (!r_small_int_p (byte)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, byte);
        return R_FAILURE;
    }

    return r_bytevector_u8_set_x
        (r, obj, r_uint_from_sexp (k), r_uint_from_sexp (byte));
}

RPrimitiveDesc bytevector_primitives [] = {
    { "make-bytevector",    np_make_bytevector,     1, 1, FALSE },
    { "bytevector-length",  np_bytevector_length,   1, 0, FALSE },
    { "list->bytevector",   np_list_to_bytevector,  1, 0, FALSE },
    { "bytevector-u8-ref",  np_bytevector_u8_ref,   2, 0, FALSE },
    { "bytevector-u8-set!", np_bytevector_u8_set_x, 3, 0, FALSE },
    { NULL }
};
