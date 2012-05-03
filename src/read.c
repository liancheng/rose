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
                return SEXP_EOF;\
            }\
            else if (TKN_FAIL == r_scanner_peek_token_id(input, context)) {\
                return SEXP_UNSPECIFIED;\
            }\
        }\
        while (0)

rsexp sexp_read_boolean(FILE* input, RContext* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_BOOLEAN != r_scanner_peek_token_id(input, context))
        return SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token(input, context);
    rsexp res = ('t' == t->text[1]) ? SEXP_TRUE : SEXP_FALSE;
    r_scanner_free_token(t);

    return res;
}

rsexp sexp_read_string(FILE* input, RContext* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_STRING != r_scanner_peek_token_id(input, context))
        return SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token(input, context);
    rsexp res = sexp_string_new((char*)t->text);
    r_scanner_free_token(t);

    return res;
}

rsexp sexp_read_symbol(FILE* input, RContext* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_IDENTIFIER != r_scanner_peek_token_id(input, context))
        return SEXP_UNSPECIFIED;

    RToken* t = r_scanner_next_token(input, context);
    rsexp res = sexp_from_symbol((char*)(t->text), context);
    r_scanner_free_token(t);

    return res;
}

rsexp sexp_read_simple_datum(FILE* input, RContext* context)
{
    rsexp res = sexp_read_boolean(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_string(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_symbol(input, context);

    return res;
}

rsexp sexp_read_abbrev(FILE* input, RContext* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    rsexp res = SEXP_UNSPECIFIED;
    rtokenid id = r_scanner_peek_token_id(input, context);

    switch (id) {
        case TKN_SINGLE_QUOTE: res = KEYWORD(QUOTE,            context); break;
        case TKN_BACK_QUOTE:   res = KEYWORD(QUASIQUOTE,       context); break;
        case TKN_COMMA:        res = KEYWORD(UNQUOTE,          context); break;
        case TKN_COMMA_AT:     res = KEYWORD(UNQUOTE_SPLICING, context); break;

        default: return res;
    }

    r_scanner_consume_token(input, context);
    rsexp sexp = sexp_read_datum(input, context);

    res = SEXP_UNSPECIFIED_P(sexp)
        ? SEXP_UNSPECIFIED
        : sexp_list(2, res, sexp);

    return res;
}

rsexp sexp_read_list(FILE* input, RContext* context)
{
    // Handle abbreviations (quote, unquote, unquote-splicing & quasiquote).
    rsexp res = sexp_read_abbrev(input, context);

    if (!SEXP_UNSPECIFIED_P(res))
        return res;

    // Consume the `('.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_LP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return SEXP_UNSPECIFIED;

    // Initialize the list to '().
    res = SEXP_NULL;

    // Read data until `.' or `)', cons the list in reverse order...
    rsexp sexp = SEXP_UNSPECIFIED;
    while (!SEXP_UNSPECIFIED_P(sexp = sexp_read_datum(input, context)))
        res = sexp_cons(sexp, res);

    // ... and then revert it.
    res = sexp_reverse(res);

    // Handle dotted improper list like (a b c . d).
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_DOT == r_scanner_peek_token_id(input, context)) {
        r_scanner_consume_token(input, context);

        rsexp last = sexp_read_datum(input, context);
        if (SEXP_UNSPECIFIED_P(last))
            return SEXP_UNSPECIFIED;

        sexp_append_x(res, last);
    }

    // Consume the `)'.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_RP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return SEXP_UNSPECIFIED;

    return res;
}

rsexp sexp_read_vector(FILE* input, RContext* context)
{
    // Consume the `#('.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_POUND_LP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return SEXP_UNSPECIFIED;

    // Read vector element into a list.
    rsexp acc = SEXP_NULL;
    rsexp sexp = SEXP_UNSPECIFIED;

    while (!SEXP_UNSPECIFIED_P(sexp = sexp_read_datum(input, context)))
        acc = sexp_cons(sexp, acc);

    // Consume the `)'.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_RP == r_scanner_peek_token_id(input, context))
        r_scanner_consume_token(input, context);
    else
        return SEXP_UNSPECIFIED;

    // Convert the list to a vector.
    return sexp_list_to_vector(sexp_reverse(acc));
}

rsexp sexp_read_compound_datum(FILE* input, RContext* context)
{
    rsexp res = sexp_read_list(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_vector(input, context);

    return res;
}

rsexp sexp_read_datum(FILE* input, RContext* context)
{
    rsexp res = sexp_read_simple_datum(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_compound_datum(input, context);

    return res;
}
