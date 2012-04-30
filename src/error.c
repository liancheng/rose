#include "rose/error.h"
#include "rose/string.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

#define SEXP_TO_ERROR(s) (((r_boxed*)s)->as.error)

r_sexp sexp_error(r_sexp message, r_sexp irritants)
{
    assert(SEXP_STRING_P(message));

    r_sexp res = (r_sexp)GC_NEW(r_boxed);

    SEXP_TYPE(res)               = SEXP_ERROR;
    SEXP_TO_ERROR(res).message   = message;
    SEXP_TO_ERROR(res).irritants = irritants;

    return res;
}

r_sexp sexp_error_object_message(r_sexp error)
{
    assert(SEXP_ERROR_P(error));
    return SEXP_TO_ERROR(error).message;
}

r_sexp sexp_error_object_irritants(r_sexp error)
{
    assert(SEXP_ERROR_P(error));
    return SEXP_TO_ERROR(error).irritants;
}
