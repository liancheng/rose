#include "context_access.h"
#include "rose/pair.h"
#include "rose/read.h"
#include "rose/scanner.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <stdlib.h>

#define RETURN_ON_EOF_OR_FAIL(input, context)\
        do {\
            if (TKN_TERMINATION == r_scanner_peek_token_id(input, context)) {\
                return R_SEXP_EOF;\
            }\
            else if (TKN_FAIL == r_scanner_peek_token_id(input, context)) {\
                return R_SEXP_UNSPECIFIED;\
            }\
        }\
        while (0)

rsexp r_read_boolean        (FILE*     input,
                             RContext* context);
rsexp r_read_symbol         (FILE*     input,
                             RContext* context);
rsexp r_read_string         (FILE*     input,
                             RContext* context);
rsexp r_read_simple_datum   (FILE*     input,
                             RContext* context);
rsexp r_read_list           (FILE*     input,
                             RContext* context);
rsexp r_read_compound_datum (FILE*     input,
                             RContext* context);

rsexp r_read_boolean(FILE* input, RContext* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_BOOLEAN != r_scanner_peek_token_id(input, context))
        return R_SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token(input, context);
    rsexp res = ('t' == t->text[1]) ? R_SEXP_TRUE : R_SEXP_FALSE;
    r_scanner_free_token(t);

    return res;
}

rsexp r_read_string(FILE* input, RContext* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_STRING != r_scanner_peek_token_id(input, context))
        return R_SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token(input, context);
    rsexp res = r_string_new((char*)t->text);
    r_scanner_free_token(t);

    return res;
}

rsexp r_read_symbol(FILE* input, RContext* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_IDENTIFIER != r_scanner_peek_token_id(input, context))
        return R_SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token(input, context);
    rsexp res = r_symbol_new((char*)(t->text), context);
    r_scanner_free_token(t);

    return res;
}

rsexp r_read_simple_datum(FILE* input, RContext* context)
{
    rsexp res = r_read_boolean(input, context);

    if (R_UNSPECIFIED_P(res))
        res = r_read_string(input, context);

    if (R_UNSPECIFIED_P(res))
        res = r_read_symbol(input, context);

    return res;
}

rsexp r_read_abbrev(FILE* input, RContext* context)
{
    rsexp res;
    rsexp sexp;
    rtokenid id;

    RETURN_ON_EOF_OR_FAIL(input, context);

    id  = r_scanner_peek_token_id(input, context);
    res = R_SEXP_UNSPECIFIED;

    switch (id) {
        case TKN_QUOTE:    res = r_keyword(QUOTE,            context); break;
        case TKN_BACKTICK: res = r_keyword(QUASIQUOTE,       context); break;
        case TKN_COMMA:    res = r_keyword(UNQUOTE,          context); break;
        case TKN_COMMA_AT: res = r_keyword(UNQUOTE_SPLICING, context); break;
    }

    if (R_UNSPECIFIED_P(res))
        return res;

    r_scanner_consume_token(input, context);
    sexp = r_read(input, context);

    if (!R_UNSPECIFIED_P(res))
        res = r_list(2, res, sexp);

    return res;
}

rsexp r_read_list(FILE* input, RContext* context)
{
    rsexp res;
    rsexp sexp;
    rsexp last;

    // Handle abbreviations (quote, unquote, unquote-splicing & quasiquote).
    res = r_read_abbrev(input, context);

    if (!R_UNSPECIFIED_P(res))
        return res;

    // Consume the `('.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_LP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return R_SEXP_UNSPECIFIED;

    // Initialize the list to '().  Read data until `.' or `)', cons the list
    // in reverse order and then revert it.
    for (res = R_SEXP_NULL;
         !R_UNSPECIFIED_P(sexp = r_read(input, context));
         res = r_cons(sexp, res))
        ;

    res = r_reverse(res);

    // Handle dotted improper list like (a b c . d).
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_DOT == r_scanner_peek_token_id(input, context)) {
        r_scanner_consume_token(input, context);

        last = r_read(input, context);
        if (R_UNSPECIFIED_P(last))
            return R_SEXP_UNSPECIFIED;

        r_append_x(res, last);
    }

    // Consume the `)'.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_RP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return R_SEXP_UNSPECIFIED;

    return res;
}

rsexp r_read_vector(FILE* input, RContext* context)
{
    rsexp acc;
    rsexp sexp;

    // Consume the `#('.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_POUND_LP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return R_SEXP_UNSPECIFIED;

    // Read vector element into a list.
    for (acc = R_SEXP_NULL;
         !R_UNSPECIFIED_P(sexp = r_read(input, context));
         acc = r_cons(sexp, acc))
        ;

    // Consume the `)'.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_RP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return R_SEXP_UNSPECIFIED;

    // Convert the list to a vector.
    return r_list_to_vector(r_reverse(acc));
}

rsexp r_read_compound_datum(FILE* input, RContext* context)
{
    rsexp res = r_read_list(input, context);

    if (R_UNSPECIFIED_P(res))
        res = r_read_vector(input, context);

    return res;
}

rsexp r_read(FILE* input, RContext* context)
{
    rsexp res = r_read_simple_datum(input, context);

    if (R_UNSPECIFIED_P(res))
        res = r_read_compound_datum(input, context);

    return res;
}
