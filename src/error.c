#include "detail/sexp.h"
#include "rose/error.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/writer.h"

#include <assert.h>

struct RError {
    R_OBJECT_HEADER
    rsexp message;
    rsexp irritants;
};

#define ERROR_FROM_SEXP(obj) (r_cast (RError*, (obj)))
#define ERROR_TO_SEXP(error) (r_cast (rsexp, (error)))

static void write_error (rsexp port, rsexp obj)
{
    assert (r_error_p (obj));

    r_format (port,
              "error: ~s, irritants: ~s",
              r_error_get_message (obj),
              r_error_get_irritants (obj));
}

static void display_error (rsexp port, rsexp obj)
{
    assert (r_error_p (obj));

    r_format (port,
              "error: ~a, irritants: ~a",
              r_error_get_message (obj),
              r_error_get_irritants (obj));
}

static RTypeDescriptor* error_type_info ()
{
    static RTypeDescriptor type = {
        .size = sizeof (RError),
        .name = "port",
        .ops = {
            .write    = write_error,
            .display  = display_error,
            .eqv_p    = NULL,
            .equal_p  = NULL,
            .mark     = NULL,
            .destruct = NULL
        }
    };

    return &type;
}

rsexp r_error_new (RState* state, rsexp message, rsexp irritants)
{
    assert (r_string_p (message));

    RError* res = r_cast (RError*,
                          r_object_new (state,
                                        R_TYPE_ERROR,
                                        error_type_info ()));

    res->message = message;
    res->irritants = irritants;

    return ERROR_TO_SEXP (res);
}

rbool r_error_p (rsexp obj)
{
    return r_type_tag (obj) == R_TYPE_ERROR;
}

rsexp r_error_get_message (rsexp error)
{
    assert (r_error_p (error));
    return ERROR_FROM_SEXP (error)->message;
}

rsexp r_error_get_irritants (rsexp error)
{
    assert (r_error_p (error));
    return ERROR_FROM_SEXP (error)->irritants;
}

void r_error_set_message_x (rsexp error, rsexp message)
{
    assert (r_error_p (error) && r_string_p (message));
    ERROR_FROM_SEXP (error)->message = message;
}

void r_error_set_irritants_x (rsexp error, rsexp irritants)
{
    assert (r_error_p (error));
    ERROR_FROM_SEXP (error)->irritants = irritants;
}
