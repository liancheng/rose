#include "rose/pair.h"
#include "rose/read.h"
#include "rose/scanner.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <stdlib.h>

#define RETURN_ON_EOF_OR_FAIL(input, context)\
        do {\
            if (TKN_TERMINATION == scanner_peek_token_id(input, context)) {\
                return SEXP_EOF;\
            }\
            else if (TKN_FAIL == scanner_peek_token_id(input, context)) {\
                return SEXP_UNSPECIFIED;\
            }\
        }\
        while (0)

r_sexp sexp_read_boolean(FILE* input, r_context* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_BOOLEAN != scanner_peek_token_id(input, context))
        return SEXP_UNSPECIFIED;

    r_token* t = scanner_next_token(input, context);
    r_sexp res = ('t' == t->text[1]) ? SEXP_TRUE : SEXP_FALSE;
    scanner_free_token(t);

    return res;
}

r_sexp sexp_read_string(FILE* input, r_context* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_STRING != scanner_peek_token_id(input, context))
        return SEXP_UNSPECIFIED;

    // TODO handle escaped characters like '\t' & '\n', etc.
    r_token* t = scanner_next_token(input, context);
    r_sexp res = sexp_string_strdup((char*)t->text);
    scanner_free_token(t);

    return res;
}

r_sexp sexp_read_symbol(FILE* input, r_context* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_IDENTIFIER != scanner_peek_token_id(input, context))
        return SEXP_UNSPECIFIED;

    r_token* t = scanner_next_token(input, context);
    r_sexp res = sexp_from_symbol((char*)(t->text), context);
    scanner_free_token(t);

    return res;
}

r_sexp sexp_read_simple_datum(FILE* input, r_context* context)
{
    r_sexp res = sexp_read_boolean(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_string(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_symbol(input, context);

    return res;
}

r_sexp sexp_read_abbrev(FILE* input, r_context* context)
{
    RETURN_ON_EOF_OR_FAIL(input, context);

    r_sexp res = SEXP_UNSPECIFIED;
    r_token_id id = scanner_peek_token_id(input, context);

    switch (id) {
        case TKN_SINGLE_QUOTE: {
            res = sexp_from_static_symbol("quote", context);
            break;
        }

        case TKN_BACK_QUOTE: {
            res = sexp_from_static_symbol("quasiquote", context);
            break;
        }

        case TKN_COMMA: {
            res = sexp_from_static_symbol("unquote", context);
            break;
        }

        case TKN_COMMA_AT: {
            res = sexp_from_static_symbol("unquote-splicing", context);
            break;
        }

        default:
            return res;
    }

    scanner_consume_token(input, context);
    r_sexp sexp = sexp_read_datum(input, context);

    res = SEXP_UNSPECIFIED_P(sexp)
          ? SEXP_UNSPECIFIED
          : sexp_list(2, res, sexp);

    return res;
}

r_sexp sexp_read_list(FILE* input, r_context* context)
{
    // Handle abbreviations (quote, unquote, unquote-splicing & quasiquote).
    r_sexp res = sexp_read_abbrev(input, context);

    if (!SEXP_UNSPECIFIED_P(res))
        return res;

    // Consume the `('.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_LP == scanner_peek_token_id(input, context))
        scanner_consume_token(input, context);
    else
        return SEXP_UNSPECIFIED;

    // Initialize the list to '().
    res = SEXP_NULL;

    // Read data until `.' or `)', cons the list in reverse order...
    r_sexp sexp = SEXP_UNSPECIFIED;
    while (!SEXP_UNSPECIFIED_P(sexp = sexp_read_datum(input, context)))
        res = sexp_cons(sexp, res);

    // ... and then revert it.
    res = sexp_reverse(res);

    // Handle dotted improper list like (a b c . d).
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_DOT == scanner_peek_token_id(input, context)) {
        scanner_consume_token(input, context);

        r_sexp last = sexp_read_datum(input, context);
        if (SEXP_UNSPECIFIED_P(last))
            return SEXP_UNSPECIFIED;

        sexp_append_x(res, last);
    }

    // Consume the `)'.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_RP == scanner_peek_token_id(input, context))
        scanner_consume_token(input, context);
    else
        return SEXP_UNSPECIFIED;

    return res;
}

r_sexp sexp_read_vector(FILE* input, r_context* context)
{
    // Consume the `#('.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_POUND_LP == scanner_peek_token_id(input, context))
        scanner_consume_token(input, context);
    else
        return SEXP_UNSPECIFIED;

    // Read vector element into a list.
    r_sexp acc = SEXP_NULL;
    r_sexp sexp = SEXP_UNSPECIFIED;

    while (!SEXP_UNSPECIFIED_P(sexp = sexp_read_datum(input, context)))
        acc = sexp_cons(sexp, acc);

    // Consume the `)'.
    RETURN_ON_EOF_OR_FAIL(input, context);

    if (TKN_RP == scanner_peek_token_id(input, context))
        scanner_consume_token(input, context);
    else
        return SEXP_UNSPECIFIED;

    // Convert the list to a vector.
    return sexp_list_to_vector(sexp_reverse(acc));
}

r_sexp sexp_read_compound_datum(FILE* input, r_context* context)
{
    r_sexp res = sexp_read_list(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_vector(input, context);

    return res;
}

r_sexp sexp_read_datum(FILE* input, r_context* context)
{
    r_sexp res = sexp_read_simple_datum(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_compound_datum(input, context);

    return res;
}
