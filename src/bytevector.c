#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/bytevector.h"
#include "rose/error.h"
#include "rose/memory.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/string.h"

#include <assert.h>
#include <string.h>

struct RBytevector {
    R_OBJECT_HEADER
    rsize  length;
    rbyte* data;
};

#define bytevector_from_sexp(obj)   (r_cast (RBytevector*, (obj)))
#define bytevector_to_sexp(bytevec) (r_cast (rsexp, (bytevec)))

static rsexp write_bytevector (RState* state, rsexp port, rsexp obj)
{
    rsize length;
    rsize i;

    length = r_uint_from_sexp (r_bytevector_length (obj));
    ensure (r_port_puts (state, port, "#u8("));

    if (length > 0u) {
        ensure (r_port_write (state, port,
                    r_bytevector_u8_ref (state, obj, 0u)));

        for (i = 1; i < length; ++i) {
            ensure (r_port_write (state, port,
                        r_bytevector_u8_ref (state, obj, i)));
            ensure (r_port_write_char (state, port, '\n'));
        }
    }

    ensure (r_port_write_char (state, port, ')'));

    return R_UNSPECIFIED;
}

static void destruct_bytevector (RState* state, RObject* obj)
{
    r_free (state, r_cast (RBytevector*, obj)->data);
}

static rbool check_index_overflow (RState* state, rsexp bv, rsize k)
{
    rsize length = r_uint_from_sexp (r_bytevector_length (bv));

    if (k < length)
        return TRUE;

    r_error_printf (state,
                    "bytevector index overflow, length: %u, index: %u",
                    length, k);

    return FALSE;
}

void init_bytevector_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RBytevector);
    type->name         = "bytevector";
    type->ops.write    = write_bytevector;
    type->ops.display  = write_bytevector;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = r_bytevector_equal_p;
    type->ops.mark     = NULL;
    type->ops.destruct = destruct_bytevector;

    state->builtin_types [R_BYTEVECTOR_TAG] = type;
}

rsexp r_bytevector_new (RState* state, rsize k, rbyte fill)
{
    RBytevector* res = r_object_new (state, RBytevector, R_BYTEVECTOR_TAG);

    if (!res)
        return r_last_error (state);

    res->length = k;
    res->data = k ? r_alloc (state, sizeof (rbyte) * k) : NULL;

    while (k--)
        res->data [k] = fill;

    return bytevector_to_sexp (res);
}

rbool r_bytevector_p (rsexp obj)
{
    return r_type_tag (obj) == R_BYTEVECTOR_TAG;
}

rsexp r_bytevector_length (rsexp obj)
{
    return r_uint_to_sexp (bytevector_from_sexp (obj)->length);
}

rsexp r_bytevector_u8_ref (RState* state, rsexp obj, rsize k)
{
    return check_index_overflow (state, obj, k)
           ? r_uint_to_sexp (bytevector_from_sexp (obj)->data [k])
           : r_last_error (state);
}

rsexp r_bytevector_u8_set_x (RState* state, rsexp obj, rsize k, rbyte byte)
{
    if (!check_index_overflow (state, obj, k))
        return r_last_error (state);

    bytevector_from_sexp (obj)->data [k] = byte;
    return R_TRUE;
}

rsexp r_list_to_bytevector (RState* state, rsexp list)
{
    rbyte byte;
    rsize k;
    rsize length;
    rsexp res;

    res = r_length (state, list);

    /* If `list' is not a proper list... */
    if (r_unspecified_p (res))
        return r_last_error (state);

    length = r_uint_from_sexp (res);
    res = r_bytevector_new (state, length, R_UNDEFINED);

    if (r_unspecified_p (res))
        return r_last_error (state);

    for (k = 0; k < length; ++k) {
        if (!r_byte_p (r_car (list)))
            return r_error_format (state,
                                   "wrong type argument, "
                                   "expecting: byte, given: ~a",
                                   r_car (list));

        byte = r_cast (rbyte, r_uint_from_sexp (r_car (list)));
        r_bytevector_u8_set_x (state, res, k, byte);
        list = r_cdr (list);
    }

    return res;
}

rbool r_bytevector_equal_p (RState* state, rsexp lhs, rsexp rhs)
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
