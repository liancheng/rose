#include "rose/pair.h"
#include "rose/read.h"
#include "rose/scanner.h"
#include "rose/symbol.h"

r_sexp sexp_read_boolean(FILE* input, r_context* context)
{
    if (TKN_BOOLEAN != scanner_peek_token_id(input, context))
        return SEXP_UNSPECIFIED;

    r_token* t = scanner_next_token(input, context);
    r_sexp res = ('t' == t->text[1]) ? SEXP_TRUE : SEXP_FALSE;
    scanner_free_token(t);

    return res;
}

r_sexp sexp_read_symbol(FILE* input, r_context* context)
{
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
        res = sexp_read_symbol(input, context);

    return res;
}

r_sexp sexp_read_abbrev(FILE* input, r_context* context)
{
    r_token_id id = scanner_peek_token_id(input, context);
    r_sexp res = SEXP_UNSPECIFIED;

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
            break;
    }

    if (!SEXP_UNSPECIFIED_P(res)) {
        scanner_consume_token(input, context);

        r_sexp sexp = sexp_read_datum(input, context);

        if (SEXP_UNSPECIFIED_P(sexp))
            res = SEXP_UNSPECIFIED;
        else
            res = sexp_cons(res, sexp_cons(sexp, SEXP_NULL));
    }

    return res;
}

r_sexp sexp_read_list(FILE* input, r_context* context)
{
    r_sexp res = SEXP_UNSPECIFIED;

    if (!SEXP_UNSPECIFIED_P(res = sexp_read_abbrev(input, context)))
        goto exit;

    if (TKN_LP == scanner_peek_token_id(input, context))
        scanner_consume_token(input, context);
    else
        goto exit;

    if (TKN_RP == scanner_peek_token_id(input, context)) {
        scanner_consume_token(input, context);
        res = SEXP_NULL;
        goto exit;
    }

    res = SEXP_NULL;
    r_sexp sexp = SEXP_UNSPECIFIED;

    while (!SEXP_UNSPECIFIED_P(sexp = sexp_read_datum(input, context)))
        res = sexp_cons(sexp, res);

    res = sexp_reverse(res);

    if (TKN_DOT == scanner_peek_token_id(input, context)) {
        scanner_consume_token(input, context);

        r_sexp last = sexp_read_datum(input, context);
        if (SEXP_UNSPECIFIED_P(last)) {
            res = SEXP_UNSPECIFIED;
            goto exit;
        }

        sexp_append_x(res, last);
    }

    if (TKN_RP == scanner_peek_token_id(input, context))
        scanner_consume_token(input, context);
    else
        res = SEXP_UNSPECIFIED;

exit:
    return res;
}

r_sexp sexp_read_compound_datum(FILE* input, r_context* context)
{
    return sexp_read_list(input, context);
}

r_sexp sexp_read_datum(FILE* input, r_context* context)
{
    if (TKN_TERMINATION == scanner_peek_token_id(input, context))
        return SEXP_EOF;

    r_sexp res = sexp_read_simple_datum(input, context);

    if (SEXP_UNSPECIFIED_P(res))
        res = sexp_read_compound_datum(input, context);

    return res;
}
