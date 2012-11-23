#include "detail/sexp.h"
#include "detail/state.h"
#include "rose/memory.h"
#include "rose/port.h"
#include "rose/string.h"

#include <string.h>

struct RString {
    R_OBJECT_HEADER
    rsize length;
    char* data;
};

#define STRING_FROM_SEXP(obj)   ((RString*) (obj))
#define STRING_TO_SEXP(string)  ((rsexp) (string))

static void write_string (rsexp port, rsexp obj)
{
    char* p;

    r_write_char (port, '"');

    for (p = STRING_FROM_SEXP (obj)->data; *p; ++p)
        if ('"' == *p)
            r_port_puts (port, "\\\"");
        else
            r_write_char (port, *p);

    r_write_char (port, '"');
}

static void display_string (rsexp port, rsexp obj)
{
    r_port_puts (port, STRING_FROM_SEXP (obj)->data);
}

static void destruct_string (RState* state, RObject* obj)
{
    RString* str = (RString*) obj;
    r_free (state, str->data);
}

static RTypeDescriptor* string_type_info ()
{
    static RTypeDescriptor type = {
        .size = sizeof (RString),
        .name = "string",
        .ops = {
            .write    = write_string,
            .display  = display_string,
            .eqv_p    = NULL,
            .equal_p  = r_string_equal_p,
            .mark     = NULL,
            .destruct = destruct_string
        }
    };

    return &type;
}

rsexp r_string_new (RState* state, rchar const* str)
{
    RString* res = (RString*) r_object_new (state,
                                            R_TYPE_STRING,
                                            string_type_info ());

    res->length = strlen (str) + 1;
    res->data = r_strdup (state, str);

    return STRING_TO_SEXP (res);
}

rbool r_string_p (rsexp obj)
{
    return r_type_tag (obj) == R_TYPE_STRING;
}

char const* r_string_to_cstr (rsexp obj)
{
    return STRING_FROM_SEXP (obj)->data;
}

rbool r_string_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    RString* lhs_str;
    RString* rhs_str;

    if (!r_string_p (lhs) || !r_string_p (rhs))
        return FALSE;

    lhs_str = STRING_FROM_SEXP (lhs);
    rhs_str = STRING_FROM_SEXP (rhs);

    return lhs_str->length == rhs_str->length
        && 0 == memcmp (lhs_str->data,
                        rhs_str->data,
                        lhs_str->length * sizeof (char));
}
