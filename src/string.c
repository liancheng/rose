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

static void r_write_string (rsexp port, rsexp obj)
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

static void r_display_string (rsexp port, rsexp obj)
{
    r_port_puts (port, STRING_FROM_SEXP (obj)->data);
}

static RType* r_string_type_info ()
{
    static RType type = {
        .size    = sizeof (RString),
        .name    = "string",
        .write   = r_write_string,
        .display = r_display_string
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
    return r_cell_p (obj) &&
           (R_SEXP_TYPE (obj) == r_string_type_info ());
}

char const* r_string_to_cstr (rsexp obj)
{
    return STRING_FROM_SEXP (obj)->data;
}
