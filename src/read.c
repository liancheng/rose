#include "scanner.h"
#include "sexp_io.h"

#include "rose/error.h"
#include "rose/pair.h"
#include "rose/read.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <stdlib.h>

static rsexp read_boolean (rsexp port, rsexp context)
{
    RETURN_ON_EOF_OR_FAIL (port, context);

    if (TKN_BOOLEAN != r_scanner_peek_id (port, context))
        return R_SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token (port, context);
    rsexp res = ('t' == t->text [1]) ? R_SEXP_TRUE : R_SEXP_FALSE;
    r_scanner_free_token (t);

    return res;
}

static rsexp read_simple_datum (rsexp port, rsexp context)
{
    rsexp res = read_boolean (port, context);

    if (r_unspecified_p (res))
        res = r_read_string (port, context);

    if (r_unspecified_p (res))
        res = r_read_symbol (port, context);

    return res;
}

static rsexp read_compound_datum (rsexp port, rsexp context)
{
    rsexp res = r_read_list (port, context);

    if (r_unspecified_p (res))
        res = r_read_vector (port, context);

    return res;
}

rsexp r_read (rsexp port, rint expect, rsexp context)
{
    rsexp res = read_simple_datum (port, context);

    if (r_unspecified_p (res))
        res = read_compound_datum (port, context);

    if (r_unspecified_p (res))
        if (expect)
            res = r_error (r_string_new ("not a datum"), R_SEXP_NULL);

    return res;
}
