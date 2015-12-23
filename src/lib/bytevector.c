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

    r_check_arg (r, k, r_small_int_p, R_ERR_WRONG_TYPE_ARG)
    r_check_arg (r, fill, r_byte_p, R_ERR_WRONG_TYPE_ARG)

    return r_bytevector_new (r, r_uint_from_sexp (k), r_uint_from_sexp (fill));
}

static rsexp np_bytevector_length (RState* r, rsexp args)
{
    rsexp obj;

    r_match_args (r, args, 1, 0, FALSE, &obj);
    r_check_arg (r, obj, r_bytevector_p, R_ERR_WRONG_TYPE_ARG);

    return r_bytevector_length (obj);
}

static rsexp np_list_to_bytevector (RState* r, rsexp args)
{
    rsexp list;

    r_match_args (r, args, 1, 0, FALSE, &list);
    r_check_arg (r, list, r_list_p, R_ERR_WRONG_TYPE_ARG);

    return r_list_to_bytevector (r, list);
}

static rsexp np_bytevector_u8_ref (RState* r, rsexp args)
{
    rsexp obj, k;

    r_match_args (r, args, 2, 0, FALSE, &obj, &k);
    r_check_arg (r, obj, r_bytevector_p, R_ERR_WRONG_TYPE_ARG);
    r_check_arg (r, k, r_small_int_p, R_ERR_WRONG_TYPE_ARG);

    return r_bytevector_u8_ref (r, obj, r_uint_from_sexp (k));
}

static rsexp np_bytevector_u8_set_x (RState* r, rsexp args)
{
    rsexp obj, k, byte;

    r_match_args (r, args, 3, 0, FALSE, &obj, &k, &byte);
    r_check_arg (r, obj, r_bytevector_p, R_ERR_WRONG_TYPE_ARG);
    r_check_arg (r, k, r_small_int_p, R_ERR_WRONG_TYPE_ARG);
    r_check_arg (r, byte, r_small_int_p, R_ERR_WRONG_TYPE_ARG);

    return r_bytevector_u8_set_x
        (r, obj, r_uint_from_sexp (k), r_uint_from_sexp (byte));
}

const RPrimitiveDesc bytevector_primitives [] = {
    { "make-bytevector",    np_make_bytevector,     1, 1, FALSE },
    { "bytevector-length",  np_bytevector_length,   1, 0, FALSE },
    { "list->bytevector",   np_list_to_bytevector,  1, 0, FALSE },
    { "bytevector-u8-ref",  np_bytevector_u8_ref,   2, 0, FALSE },
    { "bytevector-u8-set!", np_bytevector_u8_set_x, 3, 0, FALSE },
    { NULL }
};
