#include "detail/sexp.h"
#include "rose/port.h"
#include "rose/string.h"

#include <gc/gc.h>
#include <string.h>

struct RString {
    RType* type;
    rsize  length;
    char*  data;
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

static RType* r_string_type_info ()
{
    static RType type = {
        .size = sizeof (RString),
        .name = "string",
        .ops = {
            .write = write_string,
            .display = display_string
        }
    };

    return &type;
}

rsexp r_string_new (char const* str)
{
    RString* res = GC_NEW (RString);

    res->type   = r_string_type_info ();
    res->length = strlen (str) + 1;
    res->data   = GC_STRDUP (str);

    return STRING_TO_SEXP (res);
}

rbool r_string_p (rsexp obj)
{
    return r_boxed_p (obj) &&
           (R_SEXP_TYPE (obj) == r_string_type_info ());
}

char const* r_string_to_cstr (rsexp obj)
{
    return STRING_FROM_SEXP (obj)->data;
}

rbool r_string_equal_p (rsexp lhs, rsexp rhs)
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
