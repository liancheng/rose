#include "scanner.h"

#include "rose/pair.h"
#include "rose/read.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <stdlib.h>

static rsexp read_boolean(rsexp input, rsexp context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_BOOLEAN != r_scanner_peek_token_id(input, context))
        return R_SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token(input, context);
    rsexp res = ('t' == t->text[1]) ? R_SEXP_TRUE : R_SEXP_FALSE;
    r_scanner_free_token(t);

    return res;
}

static rsexp read_simple_datum(rsexp input, rsexp context)
{
    rsexp res = read_boolean(input, context);

    if (r_unspecified_p(res))
        res = r_read_string(input, context);

    if (r_unspecified_p(res))
        res = r_read_symbol(input, context);

    return res;
}

static rsexp read_compound_datum(rsexp input, rsexp context)
{
    rsexp res = r_read_list(input, context);

    if (r_unspecified_p(res))
        res = r_read_vector(input, context);

    return res;
}

rsexp r_read(rsexp input, rsexp context)
{
    rsexp res = read_simple_datum(input, context);

    if (r_unspecified_p(res))
        res = read_compound_datum(input, context);

    return res;
}
