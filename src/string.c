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

#define SEXP_TO_STRING(obj)   (*((RString*) (obj)))
#define SEXP_FROM_STRING(str) ((rsexp) (str))

static void r_write_string (rsexp port, rsexp obj, RContext* context)
{
    char* p;

    r_write_char (port, '"');

    for (p = SEXP_TO_STRING (obj).data; *p; ++p)
        if ('"' == *p)
            r_port_puts (port, "\\\"");
        else
            r_write_char (port, *p);

    r_write_char (port, '"');
}

static void r_display_string (rsexp port, rsexp obj, RContext* context)
{
    r_port_puts (port, SEXP_TO_STRING (obj).data);
}

static RType* r_string_type_info ()
{
    static RType* type = NULL;

    if (!type) {
        type = GC_NEW (RType);

        type->cell_size  = sizeof (RString);
        type->name       = "string";
        type->write_fn   = r_write_string;
        type->display_fn = r_display_string;
    }

    return type;
}

rsexp r_string_new (char const* str)
{
    RString* res = GC_NEW (RString);

    res->type   = r_string_type_info ();
    res->length = strlen (str) + 1;
    res->data   = GC_STRDUP (str);

    return SEXP_FROM_STRING (res);
}

rboolean r_string_p (rsexp obj)
{
    return r_cell_p (obj) &&
           (R_CELL_TYPE (obj) == r_string_type_info ());
}

char const* r_string_to_cstr (rsexp obj)
{
    return SEXP_TO_STRING (obj).data;
}
