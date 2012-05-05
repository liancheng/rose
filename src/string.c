#include "boxed.h"
#include "scanner.h"

#include "rose/string.h"

#include <string.h>

#define SEXP_TO_STRING(sexp) R_BOXED_VALUE(sexp).string

rboolean r_string_p(rsexp sexp)
{
    return r_boxed_get_type(sexp) == SEXP_STRING;
}

rsexp r_string_new(char const* str)
{
    int length;

    R_SEXP_NEW(res, SEXP_STRING);

    length                     = strlen(str) + 1;
    SEXP_TO_STRING(res).length = length;
    SEXP_TO_STRING(res).data   = GC_MALLOC_ATOMIC(length * sizeof(char));

    strncpy(SEXP_TO_STRING(res).data, str, length + 1);

    return res;
}

void r_write_string(rsexp output, rsexp sexp, rsexp context)
{
    r_pprintf(output, "\"%s\"", R_BOXED_VALUE(sexp).string.data);
}

rsexp r_read_string(rsexp input, rsexp context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_STRING != r_scanner_peek_token_id(input, context))
        return R_SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token(input, context);
    rsexp res = r_string_new((char*)t->text);
    r_scanner_free_token(t);

    return res;
}
