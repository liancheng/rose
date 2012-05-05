#include "boxed.h"
#include "opaque.h"

rsexp r_opaque_new(rpointer opaque)
{
    R_SEXP_NEW(res, SEXP_OPAQUE);
    R_BOXED_VALUE(res).opaque = opaque;
    return res;
}

void r_opaque_set(rsexp sexp, rpointer opaque)
{
    R_BOXED_VALUE(sexp).opaque = opaque;
}

rpointer r_opaque_get(rsexp sexp)
{
    return R_BOXED_VALUE(sexp).opaque;
}
