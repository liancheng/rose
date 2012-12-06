#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/bytevector.h"
#include "rose/memory.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"

#include <assert.h>
#include <string.h>

struct RBytevector {
    R_OBJECT_HEADER
    rsize  length;
    rbyte* data;
};

#define bytevector_from_sexp(obj)   (r_cast (RBytevector*, (obj)))
#define bytevector_to_sexp(bytevec) (r_cast (rsexp, (bytevec)))

static void write_bytevector (RState* state, rsexp port, rsexp obj)
{
    rsize length = r_bytevector_length (obj);
    rsize i;

    r_port_puts (port, "#u8(");

    if (length > 0u) {
        r_port_write (state, port, r_bytevector_u8_ref (obj, 0u));

        for (i = 1u; i < length; ++i) {
            r_port_write (state, port, r_bytevector_u8_ref (obj, i));
            r_write_char (port, '\n');
        }
    }

    r_write_char (port, ')');
}

static void destruct_bytevector (RState* state, RObject* obj)
{
    r_free (state, r_cast (RBytevector*, obj)->data);
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
        return R_FALSE;

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

rsize r_bytevector_length (rsexp obj)
{
    return bytevector_from_sexp (obj)->length;
}

rbyte r_bytevector_u8_ref (rsexp obj, rsize k)
{
    assert (r_bytevector_p (obj));
    assert (r_bytevector_length (obj) > k);
    return bytevector_from_sexp (obj)->data [k];
}

rsexp r_bytevector_u8_set_x (rsexp obj, rsize k, rbyte byte)
{
    assert (r_bytevector_p (obj));
    assert (r_bytevector_length (obj) > k);
    bytevector_from_sexp (obj)->data [k] = byte;
    return R_UNSPECIFIED;
}

rsexp r_list_to_bytevector (RState* state, rsexp list)
{
    rbyte byte;
    rsize k;
    rsize length = r_length (list);
    rsexp res = r_bytevector_new (state, length, R_UNSPECIFIED);

    for (k = 0; k < length; ++k) {
        byte = r_cast (rbyte, r_int_from_sexp (r_car (list)));
        r_bytevector_u8_set_x (res, k, byte);
        list = r_cdr (list);
    }

    return res;
}

rbool r_bytevector_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    if (!r_bytevector_p (lhs) || !r_bytevector_p (rhs))
        return FALSE;

    return r_bytevector_length (lhs) == r_bytevector_length (rhs)
        && 0 == memcmp (bytevector_from_sexp (lhs)->data,
                        bytevector_from_sexp (rhs)->data,
                        r_bytevector_length (lhs) * sizeof (rbyte));
}
