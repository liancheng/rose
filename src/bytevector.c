#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/bytevector.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/io.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/string.h"

#include <assert.h>
#include <string.h>

typedef struct RBytevector RBytevector;

struct RBytevector {
    R_OBJECT_HEADER
    rsize length;
    rbyte* data;
};

#define bytevector_from_sexp(obj)   (r_cast (RBytevector*, (obj)))
#define bytevector_to_sexp(bytevec) (r_cast (rsexp, (bytevec)))

static rsexp bytevector_write (RState* r, rsexp port, rsexp obj)
{
    rsize length;
    rsize i;
    rsexp value;

    length = r_uint_from_sexp (r_bytevector_length (obj));
    ensure (r_port_puts (r, port, "#u8("));

    if (length > 0u) {
        value = r_bytevector_u8_ref (r, obj, 0u);
        ensure (r_port_write (r, port, value));

        for (i = 1; i < length; ++i) {
            value = r_bytevector_u8_ref (r, obj, i);
            ensure (r_port_write_char (r, port, ' '));
            ensure (r_port_write (r, port, value));
        }
    }

    ensure (r_port_write_char (r, port, ')'));

    return R_UNSPECIFIED;
}

static void bytevector_finalize (RState* r, RObject* obj)
{
    RBytevector* bv = r_cast (RBytevector*, obj);

    if (bv->length)
        r_free (r, bv->data);
}

static rbool check_index_overflow (RState* r, rsexp bv, rsize k)
{
    rsize length = r_uint_from_sexp (r_bytevector_length (bv));

    if (k >= length) {
        r_error_code (r, R_ERR_INDEX_OVERFLOW);
        return FALSE;
    }

    return TRUE;
}

static rbool bytevector_equal_p (RState* r, rsexp lhs, rsexp rhs)
{
    rsize lhs_len;
    rsize rhs_len;

    if (!r_bytevector_p (lhs) || !r_bytevector_p (rhs))
        return FALSE;

    lhs_len = r_uint_from_sexp (r_bytevector_length (lhs));
    rhs_len = r_uint_from_sexp (r_bytevector_length (rhs));

    return lhs_len == rhs_len
        && 0 == memcmp (bytevector_from_sexp (lhs)->data,
                        bytevector_from_sexp (rhs)->data,
                        lhs_len * sizeof (rbyte));
}

rsexp r_bytevector_new (RState* r, rsize k, rbyte fill)
{
    RBytevector* res = r_object_new (r, RBytevector, R_TAG_BYTEVECTOR);

    if (!res)
        return R_FAILURE;

    res->length = k;
    res->data = k ? r_new_array (r, rbyte, k) : NULL;

    while (k--)
        res->data [k] = fill;

    return bytevector_to_sexp (res);
}

rbool r_bytevector_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_BYTEVECTOR;
}

rsexp r_bytevector_length (rsexp obj)
{
    return r_uint_to_sexp (bytevector_from_sexp (obj)->length);
}

rsexp r_bytevector_u8_ref (RState* r, rsexp obj, rsize k)
{
    return check_index_overflow (r, obj, k)
           ? r_uint_to_sexp (bytevector_from_sexp (obj)->data [k])
           : R_FAILURE;
}

rsexp r_bytevector_u8_set_x (RState* r, rsexp obj, rsize k, rbyte byte)
{
    if (!check_index_overflow (r, obj, k))
        return R_FAILURE;

    bytevector_from_sexp (obj)->data [k] = byte;

    return R_UNSPECIFIED;
}

rsexp r_list_to_bytevector (RState* r, rsexp list)
{
    rbyte byte;
    rsize k;
    rsize length;
    rsexp res;

    res = r_length (r, list);

    /* If `list' is not a proper list... */
    if (r_failure_p (res)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, list);
        goto exit;
    }

    r_gc_scope_open (r);

    length = r_uint_from_sexp (res);
    res = r_bytevector_new (r, length, R_UNDEFINED);

    if (r_failure_p (res))
        goto clean;

    for (k = 0; k < length; ++k) {
        if (!r_byte_p (r_car (list))) {
            r_error_code (r, R_ERR_WRONG_TYPE_ARG, list);
            res = R_FAILURE;
            goto clean;
        }

        byte = r_cast (rbyte, r_uint_from_sexp (r_car (list)));
        r_bytevector_u8_set_x (r, res, k, byte);
        list = r_cdr (list);
    }

clean:
    r_gc_scope_close_and_protect (r, res);

exit:
    return res;
}

RTypeInfo bytevector_type = {
    .size = sizeof (RBytevector),
    .name = "bytevector",
    .ops = {
        .write = bytevector_write,
        .display = bytevector_write,
        .equal_p = bytevector_equal_p,
        .finalize = bytevector_finalize
    }
};
