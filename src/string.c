#include "rose/string.h"

#include <gc/gc.h>
#include <string.h>

rsexp r_string_new(char const* str)
{
    int length;
    rsexp res;

    length                     = strlen(str) + 1;
    res                        = (rsexp)GC_NEW(RBoxed);
    SEXP_TYPE(res)             = SEXP_STRING;
    SEXP_TO_STRING(res).length = length;
    SEXP_TO_STRING(res).data   = GC_MALLOC_ATOMIC(length * sizeof(char));

    strncpy(SEXP_TO_STRING(res).data, str, length + 1);

    return res;
}
