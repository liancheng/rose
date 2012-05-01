#include "rose/error.h"
#include "rose/string.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

#define SEXP_TO_ERROR(s) (((RBoxed*)s)->as.error)

rsexp sexp_error(rsexp message, rsexp irritants)
{
    assert(SEXP_STRING_P(message));

    rsexp res = (rsexp)GC_NEW(RBoxed);

    SEXP_TYPE(res)                = SEXP_ERROR;
    SEXP_AS(res, error).message   = message;
    SEXP_AS(res, error).irritants = irritants;

    return res;
}

rsexp sexp_error_object_message(rsexp error)
{
    assert(SEXP_ERROR_P(error));
    return SEXP_AS(error, error).message;
}

rsexp sexp_error_object_irritants(rsexp error)
{
    assert(SEXP_ERROR_P(error));
    return SEXP_AS(error, error).irritants;
}
