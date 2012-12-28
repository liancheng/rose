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

static rsexp error_write (RState* state, rsexp port, rsexp obj)
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
                               r_error_object_message (obj),
                               r_error_object_irritants (obj)));

    return R_UNSPECIFIED;
}

static rsexp error_display (RState* state, rsexp port, rsexp obj)
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
                               r_error_object_message (obj),
                               r_error_object_irritants (obj)));

    return R_UNSPECIFIED;
}

static void error_mark (RState* state, rsexp obj)
{
    if (!r_inline_error_p (obj)) {
        r_gc_mark (state, error_from_sexp (obj)->message);
        r_gc_mark (state, error_from_sexp (obj)->irritants);
    }
}

void init_error_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.size         = sizeof (RError);
    type.name         = "error";
    type.ops.write    = error_write;
    type.ops.display  = error_display;
    type.ops.eqv_p    = NULL;
    type.ops.equal_p  = NULL;
    type.ops.mark     = error_mark;
    type.ops.finalize = NULL;

    init_builtin_type (state, R_TAG_INLINE_ERROR, &type);
    init_builtin_type (state, R_TAG_ERROR,        &type);
}

rsexp r_error_new (RState* state, rsexp message, rsexp irritants)
{
    RError* res = r_object_new (state, RError, R_TAG_ERROR);

    res->message = message;
    res->irritants = irritants;

    return error_to_sexp (res);
}

rbool r_error_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_ERROR
        || r_type_tag (obj) == R_TAG_INLINE_ERROR;
}

rsexp r_error_object_message (rsexp error)
{
    return error_from_sexp (error)->message;
}

rsexp r_error_object_irritants (rsexp error)
{
    return error_from_sexp (error)->irritants;
}

rsexp r_error_printf (RState* state, rconstcstring format, ...)
{
    va_list args;
    rsexp   message;
    rsexp   res;

    r_gc_scope_open (state);

    va_start (args, format);
    message = r_string_vprintf (state, format, args);
    res = r_set_last_error_x (state, r_error_new (state, message, R_NULL));
    va_end (args);

    r_gc_scope_close_and_protect (state, res);

    return res;
}

rsexp r_error_format (RState* state, rconstcstring format, ...)
{
    va_list args;
    rsexp   message;
    rsexp   res;

    r_gc_scope_open (state);

    va_start (args, format);
    message = r_string_vformat (state, format, args);
    res = r_set_last_error_x (state, r_error_new (state, message, R_NULL));
    va_end (args);

    r_gc_scope_close_and_protect (state, res);

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

void r_raise (RState* state)
{
    if (state->error_jmp)
        longjmp (state->error_jmp->buf, 1);
    else
        abort ();
}

rsexp r_inherit_errno_x (RState* state, rint errnum)
{
    rchar buffer [BUFSIZ];
    rsexp error;

    r_gc_scope_open (state);

    strerror_r (errnum, buffer, BUFSIZ);
    error = r_error_new (state, r_string_new (state, buffer), R_NULL);
    r_set_last_error_x (state, error);

    r_gc_scope_close_and_protect (state, error);

    return error;
}

rsexp wrong_type_arg (RState* state, rconstcstring expected, rsexp given)
{
    return r_error_format (state,
                           "wrong type argument, "
                           "expecting: ~w, given: ~s",
                           r_string_new (state, expected),
                           given);
}

rsexp bad_syntax (RState* state, rsexp expr)
{
    return r_error_format (state, "bad syntax ~s", expr);
}

rsexp bad_variable (RState* state, rsexp var, rsexp expr)
{
    return r_error_format (state, "bad variable ~s in ~s", var, expr);
}

rsexp bad_formals (RState* state, rsexp formals, rsexp expr)
{
    return r_error_format (state, "bad formals ~s in ~s", formals, expr);
}
