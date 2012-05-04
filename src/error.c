#include "boxed.h"

#include "rose/error.h"
#include "rose/string.h"

#include <assert.h>
#include <gc/gc.h>
#include <stdarg.h>

#define SEXP_TO_ERROR(s) R_BOXED_VALUE(s).error

rboolean r_error_p(rsexp sexp)
{
    return r_boxed_get_type(sexp) == SEXP_ERROR;
}

rsexp r_error(rsexp message, rsexp irritants)
{
    rsexp res;

    assert(r_string_p(message));

    res = (rsexp)GC_NEW(RBoxed);
    SEXP_TO_ERROR(res).message = message;
    SEXP_TO_ERROR(res).irritants = irritants;

    r_boxed_set_type(res, SEXP_ERROR);

    return res;
}

rsexp r_error_message(rsexp error)
{
    assert(r_error_p(error));
    return SEXP_TO_ERROR(error).message;
}

rsexp r_error_irritants(rsexp error)
{
    assert(r_error_p(error));
    return SEXP_TO_ERROR(error).irritants;
}

void r_error_set_message_x(rsexp error, rsexp message)
{
    assert(r_error_p(error));
    assert(r_string_p(message));
    SEXP_TO_ERROR(error).message = message;
}

void r_error_set_irritants_x(rsexp error, rsexp irritants)
{
    assert(r_error_p(error));
    SEXP_TO_ERROR(error).irritants = irritants;
}
