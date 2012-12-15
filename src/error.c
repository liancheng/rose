#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/port.h"
#include "rose/string.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct RError RError;

struct RError {
    R_OBJECT_HEADER
    rsexp message;
    rsexp irritants;
};

#define error_from_sexp(obj) (r_cast (RError*, (obj)))
#define error_to_sexp(error) (r_cast (rsexp, (error)))

static rsexp write_error (RState* state, rsexp port, rsexp obj)
{
    static rconstcstring message;

    switch (obj) {
        case R_ERROR_OOM:
            message = "(error out-of-memory)";
            break;

        default:
            message = "(error unknown)";
            break;
    }

    if (r_inline_error_p (obj))
        ensure (r_port_puts (state, port, message));
    else
        ensure (r_port_format (state,
                               port,
                               "(error (message ~s) (irritants ~s))",
                               r_error_get_message (obj),
                               r_error_get_irritants (obj)));

    return R_UNSPECIFIED;
}

static rsexp display_error (RState* state, rsexp port, rsexp obj)
{
    static rconstcstring message;

    switch (obj) {
        case R_ERROR_OOM:
            message = "out of memory";
            break;

        default:
            message = "unknown error";
            break;
    }

    if (r_inline_error_p (obj))
        ensure (r_port_puts (state, port, message));
    else
        ensure (r_port_format (state,
                               port,
                               "message: ~a; irritants: ~a",
                               r_error_get_message (obj),
                               r_error_get_irritants (obj)));

    return R_UNSPECIFIED;
}

void init_error_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RError);
    type->name         = "error";
    type->ops.write    = write_error;
    type->ops.display  = display_error;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.mark     = NULL;
    type->ops.destruct = NULL;

    state->builtin_types [R_ERROR_TAG] = type;
}

rsexp r_error_new (RState* state, rsexp message, rsexp irritants)
{
    RError* res = r_object_new (state, RError, R_ERROR_TAG);

    res->message = message;
    res->irritants = irritants;

    return error_to_sexp (res);
}

rbool r_error_p (rsexp obj)
{
    return r_type_tag (obj) == R_ERROR_TAG
        || r_type_tag (obj) == R_INLINE_ERROR_TAG;
}

rsexp r_error_get_message (rsexp error)
{
    return error_from_sexp (error)->message;
}

rsexp r_error_get_irritants (rsexp error)
{
    return error_from_sexp (error)->irritants;
}

void r_error_set_message_x (rsexp error, rsexp message)
{
    error_from_sexp (error)->message = message;
}

void r_error_set_irritants_x (rsexp error, rsexp irritants)
{
    error_from_sexp (error)->irritants = irritants;
}

rsexp r_error_printf (RState* state, rconstcstring format, ...)
{
    va_list args;
    rsexp   message;
    rsexp   res;

    va_start (args, format);
    message = r_string_vprintf (state, format, args);
    res = r_set_last_error_x (state, r_error_new (state, message, R_NULL));
    va_end (args);

    return res;
}

rsexp r_error_format (RState* state, rconstcstring format, ...)
{
    va_list args;
    rsexp   message;
    rsexp   res;

    va_start (args, format);
    message = r_string_vformat (state, format, args);
    res = r_set_last_error_x (state, r_error_new (state, message, R_NULL));
    va_end (args);

    return res;
}

rsexp r_error (RState* state, rconstcstring message)
{
    rsexp error = r_error_new (state, r_string_new (state, message), R_NULL);
    return r_set_last_error_x (state, error);
}

rsexp r_last_error (RState* state)
{
    return state->last_error;
}

rsexp r_set_last_error_x (RState* state, rsexp error)
{
    rsexp old = state->last_error;
    state->last_error = error;
    return old;
}

rsexp r_clear_last_error_x (RState* state)
{
    rsexp old = state->last_error;
    state->last_error = R_UNDEFINED;
    return old;
}

rsexp r_inherit_errno_x (RState* state, rint errnum)
{
    rchar buffer [BUFSIZ];
    rsexp error;

    strerror_r (errnum, buffer, BUFSIZ);
    error = r_error_new (state, r_string_new (state, buffer), R_NULL);
    r_set_last_error_x (state, error);

    return error;
}

void r_raise (RState* state)
{
    if (state->error_jmp)
        longjmp (state->error_jmp->buf, 1);
    else
        abort ();
}

rsexp error_wrong_type_arg (RState* state, rconstcstring expected, rsexp given)
{
    return r_error_format (state,
                           "wrong type argument, "
                           "expecting: ~w, given: ~s",
                           r_string_new (state, expected),
                           given);
}
