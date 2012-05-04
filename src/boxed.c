#include "boxed.h"

rint r_boxed_get_type(rsexp sexp)
{
    return ((RBoxed*)sexp)->type;
}

void r_boxed_set_type(rsexp sexp, rint type)
{
    ((RBoxed*)sexp)->type = type;
}
