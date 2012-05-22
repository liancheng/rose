#include "cell.h"
#include "scanner.h"

#include "rose/string.h"

#include <string.h>

#define SEXP_TO_STRING(obj) R_CELL_VALUE (obj).string

char* r_strdup (char const* str)
{
    char* res = GC_MALLOC_ATOMIC (strlen (str) * sizeof (char));
    strcpy (res, str);
    return res;
}

rboolean r_string_p (rsexp obj)
{
    return r_cell_p (obj) &&
           r_cell_get_type (obj) == SEXP_STRING;
}

rsexp r_string_new (char const* str)
{
    R_SEXP_NEW (res, SEXP_STRING);

    SEXP_TO_STRING (res).length = strlen (str) + 1;
    SEXP_TO_STRING (res).data   = r_strdup (str);

    return res;
}

char const* r_string_cstr (rsexp obj)
{
    return SEXP_TO_STRING (obj).data;
}

rsexp r_read_string (rsexp input, rsexp context)
{
    RETURN_ON_EOF_OR_FAIL (input, context);

    if (TKN_STRING != r_scanner_peek_id (input, context))
        return R_SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token (input, context);
    rsexp res = r_string_new ((char*) t->text);
    r_scanner_free_token (t);

    return res;
}

void r_write_string (rsexp port, rsexp obj, rsexp context)
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

void r_display_string (rsexp port, rsexp obj, rsexp context)
{
    r_port_puts (port, SEXP_TO_STRING (obj).data);
}
