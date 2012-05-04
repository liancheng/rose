#include "boxed.h"
#include "opaque.h"

#include <gc/gc.h>

rsexp r_opaque_new(rpointer opaque)
{
    rsexp res;

    res = (rsexp)GC_NEW(RBoxed);
    R_BOXED_VALUE(res).opaque = opaque;
    r_boxed_set_type(res, SEXP_OPAQUE);

    return res;
}

void r_opaque_set_x(rsexp sexp, rpointer opaque)
{
    R_BOXED_VALUE(sexp).opaque = opaque;
}

rpointer r_opaque_get(rsexp sexp)
{
    return R_BOXED_VALUE(sexp).opaque;
}
