#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/state.h"
#include "detail/string.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/io.h"
#include "rose/pair.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct RError RError;

struct RError {
    R_OBJECT_HEADER
    rsexp message;
    rsexp irritants;
};

static RErrorDesc error_desc [] = {
#define ERROR_DESC(group, enum_name, desc)\
        { group, enum_name, desc },
#include "rose/error_desc.inc"
#undef ERROR_DESC
};

#define error_from_sexp(obj) (r_cast (RError*, (obj)))
#define error_to_sexp(error) (r_cast (rsexp, (error)))

static rsexp error_write (RState* r, rsexp port, rsexp obj)
{
    ensure (r_port_format (r,
                           port,
                           "#<error \"~a\" ~s>",
                           r_error_object_message (obj),
                           r_error_object_irritants (obj)));

    return R_UNSPECIFIED;
}

static void error_mark (RState* r, rsexp obj)
{
    r_gc_mark (r, error_from_sexp (obj)->message);
    r_gc_mark (r, error_from_sexp (obj)->irritants);
}

rsexp r_error_new (RState* r, rsexp message, rsexp irritants)
{
    RError* res = r_object_new (r, RError, R_TAG_ERROR);

    res->message = message;
    res->irritants = irritants;

    return error_to_sexp (res);
}

rbool r_error_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_ERROR;
}

rsexp r_error_object_message (rsexp error)
{
    return error_from_sexp (error)->message;
}

rsexp r_error_object_irritants (rsexp error)
{
    return error_from_sexp (error)->irritants;
}

rsexp r_error_printf (RState* r, rconstcstring format, ...)
{
    va_list args;
    rsexp message;
    rsexp res;

    r_gc_scope_open (r);
    va_start (args, format);

    message = r_string_vprintf (r, format, args);

    if (r_failure_p (message)) {
        res = r_last_error (r);
        goto exit;
    }

    res = r_error_new (r, message, R_NULL);

    if (r_failure_p (res)) {
        res = r_last_error (r);
        goto exit;
    }

    r_set_last_error_x (r, res);

exit:
    va_end (args);
    r_gc_scope_close_and_protect (r, res);

    return res;
}

rsexp r_error_format (RState* r, rconstcstring format, ...)
{
    va_list args;
    rsexp   message;
    rsexp   res;

    r_gc_scope_open (r);
    va_start (args, format);

    message = r_string_vformat (r, format, args);

    if (r_failure_p (message)) {
        res = r_last_error (r);
        goto exit;
    } 

    res = r_error_new (r, message, R_NULL);

    if (r_failure_p (res)) {
        res = r_last_error (r);
        goto exit;
    }

    r_set_last_error_x (r, res);

exit:
    va_end (args);
    r_gc_scope_close_and_protect (r, res);

    return res;
}

rsexp r_error (RState* r, rconstcstring message)
{
    rsexp string = r_string_new (r, message);
    rsexp error = r_error_new (r, string, R_NULL);
    return r_set_last_error_x (r, error);
}

rsexp r_verror_code (RState* r, RErrorCode error_code, va_list args)
{
    rsexp message;
    rsexp error;

    assert (0 <= error_code && error_code < R_ERR_END);

    message = r_string_vformat (r, error_desc [error_code].desc, args);
    error = r_error_new (r, message, R_NULL);

    return r_set_last_error_x (r, error);
}

rsexp r_error_code (RState* r, RErrorCode error_code, ...)
{
    va_list args;
    rsexp res;

    r_gc_scope_open (r);
    va_start (args, error_code);

    res = r_verror_code (r, error_code, args);

    va_end (args);
    r_gc_scope_close_and_protect (r, res);

    return res;
}

rsexp r_last_error (RState* r)
{
    return r->last_error;
}

rsexp r_set_last_error_x (RState* r, rsexp error)
{
    r->last_error = error;
    return error;
}

rsexp r_clear_last_error_x (RState* r)
{
    rsexp old = r->last_error;
    r->last_error = R_UNDEFINED;
    return old;
}

void r_raise (RState* r)
{
    if (r->error_jmp)
        longjmp (r->error_jmp->buf, 1);
    else
        abort ();
}

rsexp r_inherit_errno_x (RState* r, rint errnum)
{
    rchar buffer [BUFSIZ];
    rsexp error;

    r_gc_scope_open (r);

    strerror_r (errnum, buffer, BUFSIZ);
    error = r_error_new (r, r_string_new (r, buffer), R_NULL);
    r_set_last_error_x (r, error);

    r_gc_scope_close_and_protect (r, error);

    return error;
}

void r_error_no_memory (RState* r)
{
    r_set_last_error_x (r, r->oom_error);
}

RTypeInfo error_type = {
    .size = sizeof (RError),
    .name = "error",
    .ops = {
        .write = error_write,
        .display = error_write,
        .mark = error_mark
    }
};
