#include "boxed.h"

#include <assert.h>
#include <gc/gc.h>

rint r_boxed_get_type(rsexp sexp)
{
    assert(R_BOXED_P(sexp));
    return ((RBoxed*)sexp)->type;
}

void r_boxed_set_type(rsexp sexp, rint type)
{
    assert(R_BOXED_P(sexp));
    ((RBoxed*)sexp)->type = type;
}
