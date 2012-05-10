#include "boxed.h"

#include "rose/error.h"
#include "rose/string.h"

#include <assert.h>

#define SEXP_TO_ERROR(s) R_BOXED_VALUE (s).error

rboolean r_error_p (rsexp obj)
{
    return r_boxed_p (obj) &&
           r_boxed_get_type (obj) == SEXP_ERROR;
}

rsexp r_error (rsexp message, rsexp irritants)
{
    assert (r_string_p (message));

    R_SEXP_NEW (res, SEXP_ERROR);

    SEXP_TO_ERROR (res).message = message;
    SEXP_TO_ERROR (res).irritants = irritants;

    return res;
}

rsexp r_error_message (rsexp error)
{
    assert (r_error_p (error));
    return SEXP_TO_ERROR (error).message;
}

rsexp r_error_irritants (rsexp error)
{
    assert (r_error_p (error));
    return SEXP_TO_ERROR (error).irritants;
}

void r_error_set_message (rsexp error, rsexp message)
{
    assert (r_error_p (error));
    assert (r_string_p (message));
    SEXP_TO_ERROR (error).message = message;
}

void r_error_set_irritants (rsexp error, rsexp irritants)
{
    assert (r_error_p (error));
    SEXP_TO_ERROR (error).irritants = irritants;
}
