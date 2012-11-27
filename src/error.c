#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/error.h"
#include "rose/memory.h"
#include "rose/port.h"
#include "rose/string.h"

#include <assert.h>

struct RError {
    R_OBJECT_HEADER
    rsexp message;
    rsexp irritants;
};

#define ERROR_FROM_SEXP(obj) (r_cast (RError*, (obj)))
#define ERROR_TO_SEXP(error) (r_cast (rsexp, (error)))

static void write_error (RState* state, rsexp port, rsexp obj)
{
    assert (r_error_p (obj));

    r_port_format (state,
                   port,
                   "error: ~s, irritants: ~s",
                   r_error_get_message (obj),
                   r_error_get_irritants (obj));
}

static void display_error (RState* state, rsexp port, rsexp obj)
{
    assert (r_error_p (obj));

    r_port_format (state,
                   port,
                   "error: ~a, irritants: ~a",
                   r_error_get_message (obj),
                   r_error_get_irritants (obj));
}

RTypeInfo* init_error_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RError);
    type->name         = "port";
    type->ops.write    = write_error;
    type->ops.display  = display_error;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.mark     = NULL;
    type->ops.destruct = NULL;

    return type;
}

rsexp r_error_new (RState* state, rsexp message, rsexp irritants)
{
    assert (r_string_p (message));

    RError* res = r_object_new (state, RError, R_ERROR_TAG);

    res->message = message;
    res->irritants = irritants;

    return ERROR_TO_SEXP (res);
}

rbool r_error_p (rsexp obj)
{
    return r_type_tag (obj) == R_ERROR_TAG;
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

void r_error (RState* state, rconstcstring message)
{
    rsexp e = r_error_new (state, r_string_new (state, message), R_NULL);
    r_raise (state, e);
}
