#include "detail/sexp.h"
#include "detail/state.h"
#include "detail/string.h"
#include "rose/io.h"

#include <assert.h>
#include <string.h>

static rsexp string_write (RState* r, rsexp port, rsexp obj)
{
    rcstring p;

    ensure (r_port_write_char (r, port, '"'));

    for (p = string_from_sexp (obj)->data; *p; ++p)
        if ('"' == *p)
            ensure (r_port_puts (r, port, "\\\""));
        else
            ensure (r_port_write_char (r, port, *p));

    ensure (r_port_write_char (r, port, '"'));

    return R_UNSPECIFIED;
}

static rsexp string_display (RState* r, rsexp port, rsexp obj)
{
    return r_port_puts (r, port, string_from_sexp (obj)->data);
}

static void string_finalize (RState* r, RObject* obj)
{
    RString* str = r_cast (RString*, obj);
    r_free (r, str->data);
}

static rcstring cstring_dup (RState* r, rconstcstring str)
{
    rsize    size = strlen (str);
    rcstring res  = r_cast (rcstring, r_new0_array (r, rchar, size + 1));

    if (res)
        memcpy (res, str, size + 1);

    return res;
}

rsexp r_string_new (RState* r, rconstcstring str)
{
    RString* res = r_object_new (r, RString, R_TAG_STRING);

    if (!res)
        return R_FAILURE;

    res->length = strlen (str);
    res->data = cstring_dup (r, str);

    return string_to_sexp (res);
}

rbool r_string_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_STRING;
}

rconstcstring r_string_to_cstr (rsexp obj)
{
    return string_from_sexp (obj)->data;
}

rbool r_string_equal_p (RState* r, rsexp lhs, rsexp rhs)
{
    RString* lhs_str;
    RString* rhs_str;

    if (!r_string_p (lhs) || !r_string_p (rhs))
        return FALSE;

    lhs_str = string_from_sexp (lhs);
    rhs_str = string_from_sexp (rhs);

    return lhs_str->length == rhs_str->length
        && 0 == memcmp (lhs_str->data,
                        rhs_str->data,
                        lhs_str->length * sizeof (char));
}

rsize r_string_length_by_byte (rsexp obj)
{
    assert (r_string_p (obj));
    return string_from_sexp (obj)->length;
}

rsize r_string_length (rsexp obj)
{
    assert (r_string_p (obj));
    return string_from_sexp (obj)->length;
}

rsexp r_string_vformat (RState* r, rconstcstring format, va_list args)
{
    rsexp port;
    rsexp res;

    r_gc_scope_open (r);

    port = r_open_output_string (r);
    r_port_vformat (r, port, format, args);
    res = r_get_output_string (r, port);

    r_gc_scope_close_and_protect (r, res);

    return res;
}

rsexp r_string_format (RState* r, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_string_vformat (r, format, args);
    va_end (args);

    return res;
}

rsexp r_string_vprintf (RState* r, rconstcstring format, va_list args)
{
    rsexp port;
    rsexp res;

    port = r_open_output_string (r);

    if (r_failure_p (port))
        goto exit;

    if (r_failure_p (r_port_vprintf (r, port, format, args))) {
        res = R_FAILURE;
        goto clean;
    }

    res = r_get_output_string (r, port);

    if (r_failure_p (res))
        goto clean;

clean:
    r_close_port (port);

exit:
    return res;
}

rsexp r_string_printf (RState* r, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_string_vprintf (r, format, args);
    va_end (args);

    return res;
}

RTypeInfo string_type = {
    .size = sizeof (RString),
    .name = "string",
    .ops = {
        .write = string_write,
        .display = string_display,
        .equal_p = r_string_equal_p,
        .finalize = string_finalize
    }
};
