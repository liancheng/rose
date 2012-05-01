#include "rose/string.h"

#include <gc/gc.h>
#include <string.h>

rsexp sexp_string_strdup(char const* str)
{
    int length = strlen(str);
    rsexp res = (rsexp)GC_NEW(RBoxed);

    SEXP_TYPE(res)              = SEXP_STRING;
    SEXP_AS(res, string).length = length;
    SEXP_AS(res, string).data   = GC_MALLOC_ATOMIC((length + 1) * sizeof(char));

    strncpy(SEXP_AS(res, string).data, str, length + 1);

    return res;
}
