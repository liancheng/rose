#include "detail/sexp.h"
#include "rose/error.h"
#include "rose/memory.h"
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

RTypeInfo* init_error_type_info (RState* state)
{
    RTypeInfo* type = R_NEW0 (state, RTypeInfo);

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

    RObject* obj = r_object_new (state, R_ERROR_TAG);
    RError* res = r_cast (RError*, obj);

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
