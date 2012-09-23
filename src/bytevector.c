#include "detail/sexp.h"
#include "rose/bytevector.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/writer.h"

#include <assert.h>
#include <gc/gc.h>

struct RBytevector {
    RType* type;
    rsize  length;
    rbyte* data;
};

#define BYTEVECTOR_FROM_SEXP(obj)   ((RBytevector*) (obj))
#define BYTEVECTOR_TO_SEXP(bytevec) ((rsexp) (bytevec))

static void write_bytevector (rsexp port, rsexp obj)
{
    rsize i;
    RBytevector* vec = BYTEVECTOR_FROM_SEXP (obj);

    r_port_puts (port, "#u8(");

    if (vec->length > 0u) {
        r_write (port, r_int_to_sexp (vec->data [0u]));

        for (i = 1u; i < vec->length; ++i)
            r_format (port, " ~s", r_int_to_sexp (vec->data [i]));
    }

    r_write_char (port, ')');
}

static RType* bytevector_type_info ()
{
    static RType type = {
        .size    = sizeof (RBytevector),
        .name    = "bytevector",
        .write   = write_bytevector,
        .display = write_bytevector
    };

    return &type;
}

rsexp r_bytevector_new (rsize k, rbyte fill)
{
    RBytevector* res = GC_NEW (RBytevector);

    res->type   = bytevector_type_info ();
    res->length = k;
    res->data   = k ? GC_MALLOC_ATOMIC (k * sizeof (rbyte)) : NULL;

    while (k--)
        res->data [k] = fill;

    return BYTEVECTOR_TO_SEXP (res);
}

rbool r_bytevector_p (rsexp obj)
{
    return r_boxed_p (obj) &&
           (R_SEXP_TYPE (obj) == bytevector_type_info ());
}

rsize r_bytevector_length (rsexp obj)
{
    assert (r_bytevector_p (obj));
    return BYTEVECTOR_FROM_SEXP (obj)->length;
}

rbyte r_bytevector_ref (rsexp obj, rsize k)
{
    assert (r_bytevector_p (obj));
    assert (r_bytevector_length (obj) > k);
    return BYTEVECTOR_FROM_SEXP (obj)->data [k];
}

rsexp r_bytevector_set_x (rsexp obj, rsize k, rbyte byte)
{
    assert (r_bytevector_p (obj));
    assert (r_bytevector_length (obj) > k);

    BYTEVECTOR_FROM_SEXP (obj)->data [k] = byte;

    return R_UNSPECIFIED;
}

rsexp r_list_to_bytevector (rsexp list)
{
    rsize length = r_length (list);
    rsexp res = r_bytevector_new (length, R_UNSPECIFIED);
    rbyte byte;
    rsize k;

    for (k = 0; k < length; ++k) {
        byte = (rbyte) r_int_from_sexp (r_car (list));
        r_bytevector_set_x (res, k, byte);
        list = r_cdr (list);
    }

    return res;
}
