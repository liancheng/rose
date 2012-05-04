#include "rose/error.h"
#include "rose/string.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

rsexp r_error(rsexp message, rsexp irritants)
{
    rsexp res;

    assert(SEXP_STRING_P(message));

    res                          = (rsexp)GC_NEW(RBoxed);
    SEXP_TYPE(res)               = SEXP_ERROR;
    SEXP_TO_ERROR(res).message   = message;
    SEXP_TO_ERROR(res).irritants = irritants;

    return res;
}

rsexp r_error_message(rsexp error)
{
    assert(SEXP_ERROR_P(error));
    return SEXP_TO_ERROR(error).message;
}

rsexp r_error_irritants(rsexp error)
{
    assert(SEXP_ERROR_P(error));
    return SEXP_TO_ERROR(error).irritants;
}

void r_error_set_message_x(rsexp error, rsexp message)
{
    assert(SEXP_ERROR_P(error));
    SEXP_TO_ERROR(error).message = message;
}

void r_error_set_irritants_x(rsexp error, rsexp irritants)
{
    assert(SEXP_ERROR_P(error));
    SEXP_TO_ERROR(error).irritants = irritants;
}
