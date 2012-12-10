#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/memory.h"
#include "rose/port.h"
#include "rose/string.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

struct RString {
    R_OBJECT_HEADER
    rsize    length;
    rcstring data;
};

#define string_from_sexp(obj)   (r_cast (RString*, (obj)))
#define string_to_sexp(string)  (r_cast (rsexp, (string)))

static void write_string (RState* state, rsexp port, rsexp obj)
{
    rcstring p;

    r_write_char (port, '"');

    for (p = string_from_sexp (obj)->data; *p; ++p)
        if ('"' == *p)
            r_port_puts (port, "\\\"");
        else
            r_write_char (port, *p);

    r_write_char (port, '"');
}

static void display_string (RState* state, rsexp port, rsexp obj)
{
    r_port_puts (port, string_from_sexp (obj)->data);
}

static void destruct_string (RState* state, RObject* obj)
{
    RString* str = r_cast (RString*, obj);
    r_free (state, str->data);
}

void init_string_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RString);
    type->name         = "string";
    type->ops.write    = write_string;
    type->ops.display  = display_string;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = r_string_equal_p;
    type->ops.mark     = NULL;
    type->ops.destruct = destruct_string;

    state->builtin_types [R_STRING_TAG] = type;
}

rsexp r_string_new (RState* state, rconstcstring str)
{
    RString* res = r_object_new (state, RString, R_STRING_TAG);

    if (!res)
        return R_FALSE;

    res->length = strlen (str);
    res->data = cstring_dup (state, str);

    return string_to_sexp (res);
}

rbool r_string_p (rsexp obj)
{
    return r_type_tag (obj) == R_STRING_TAG;
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

rint r_string_byte_count (rsexp obj)
{
    assert (r_string_p (obj));
    return string_from_sexp (obj)->length;
}

rint r_string_length (rsexp obj)
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

    port = r_open_output_string (state);

    if (r_port_vprintf (port, format, args) < 0)
        return R_UNSPECIFIED;

    res = r_get_output_string (state, port);

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
