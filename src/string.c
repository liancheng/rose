#include "boxed.h"

#include "rose/string.h"

#include <gc/gc.h>
#include <string.h>

#define SEXP_TO_STRING(s) R_BOXED_VALUE(s).string

rboolean r_string_p(rsexp sexp)
{
    return r_boxed_get_type(sexp) == SEXP_STRING;
}

rsexp r_string_new(char const* str)
{
    int length;
    rsexp res;

    length = strlen(str) + 1;
    res = (rsexp)GC_NEW(RBoxed);
    SEXP_TO_STRING(res).length = length;
    SEXP_TO_STRING(res).data = GC_MALLOC_ATOMIC(length * sizeof(char));

    r_boxed_set_type(res, SEXP_STRING);
    strncpy(SEXP_TO_STRING(res).data, str, length + 1);

    return res;
}

void r_write_string(FILE* output, rsexp sexp, RContext* context)
{
    fprintf(output, "\"%s\"", R_BOXED_VALUE(sexp).string.data);
}
