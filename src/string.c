#include "rose/string.h"

#include <gc/gc.h>
#include <string.h>

#define SEXP_TO_STRING(s) (((RBoxed*)s)->as.string)

rsexp sexp_string_strdup(char const* str)
{
    int length = strlen(str);
    rsexp res = (rsexp)GC_NEW(RBoxed);

    SEXP_TYPE(res)             = SEXP_STRING;
    SEXP_TO_STRING(res).length = length;
    SEXP_TO_STRING(res).data   = GC_MALLOC_ATOMIC((length + 1) * sizeof(char));

    strncpy(SEXP_TO_STRING(res).data, str, length + 1);

    return res;
}
