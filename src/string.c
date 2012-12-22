#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/gc.h"
#include "rose/port.h"
#include "rose/string.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

typedef struct RString RString;

struct RString {
    R_OBJECT_HEADER
    rsize    length;
    rcstring data;
};

#define string_from_sexp(obj)   (r_cast (RString*, (obj)))
#define string_to_sexp(string)  (r_cast (rsexp, (string)))

static rsexp string_write (RState* state, rsexp port, rsexp obj)
{
    rcstring p;

    ensure (r_port_write_char (state, port, '"'));

    for (p = string_from_sexp (obj)->data; *p; ++p)
        if ('"' == *p)
            ensure (r_port_puts (state, port, "\\\""));
        else
            ensure (r_port_write_char (state, port, *p));

    ensure (r_port_write_char (state, port, '"'));

    return R_UNSPECIFIED;
}

static rsexp string_display (RState* state, rsexp port, rsexp obj)
{
    return r_port_puts (state, port, string_from_sexp (obj)->data);
}

static void string_finalize (RState* state, RObject* obj)
{
    RString* str = r_cast (RString*, obj);
    r_free (state, str->data);
}

static rcstring cstring_dup (RState* state, rconstcstring str)
{
    rsize    size = strlen (str);
    rcstring res  = r_cast (rcstring, r_new0_array (state, rchar, size + 1));

    if (res)
        memcpy (res, str, size + 1);

    return res;
}

void init_string_type_info (RState* state)
{
    RTypeInfo type = { 0 };

    type.size         = sizeof (RString);
    type.name         = "string";
    type.ops.write    = string_write;
    type.ops.display  = string_display;
    type.ops.eqv_p    = NULL;
    type.ops.equal_p  = r_string_equal_p;
    type.ops.mark     = NULL;
    type.ops.finalize = string_finalize;

    init_builtin_type (state, R_TAG_STRING, &type);
}

rsexp r_string_new (RState* state, rconstcstring str)
{
    RString* res = r_object_new (state, RString, R_TAG_STRING);

    if (!res)
        return r_last_error (state);

    res->length = strlen (str);
    res->data = cstring_dup (state, str);

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

rbool r_string_equal_p (RState* state, rsexp lhs, rsexp rhs)
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

rsexp r_string_vformat (RState* state, rconstcstring format, va_list args)
{
    rsexp port;
    rsexp res;

    port = r_open_output_string (state);
    r_port_vformat (state, port, format, args);
    res = r_get_output_string (state, port);

    return res;
}

rsexp r_string_format (RState* state, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_string_vformat (state, format, args);
    va_end (args);

    return res;
}

rsexp r_string_vprintf (RState* state, rconstcstring format, va_list args)
{
    rsexp port;
    rsexp res;

    ensure (port = r_open_output_string (state));

    if (r_failure_p (r_port_vprintf (state, port, format, args))) {
        res = R_FAILURE;
        goto exit;
    }

    res = r_get_output_string (state, port);

    if (r_failure_p (res))
        goto exit;

exit:
    r_close_port (port);

    return res;
}

rsexp r_string_printf (RState* state, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_string_vprintf (state, format, args);
    va_end (args);

    return res;
}
