#include "scanner.h"

#include "rose/error.h"
#include "rose/pair.h"
#include "rose/parser.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <gc/gc.h>
#include <setjmp.h>

struct RParserState {
    rsexp     context;
    rsexp     input_port;
    rsexp     tree;
    rsexp     error_type;
    rsexp     last_error;
    RScanner* scanner;
    jmp_buf   jmp;
};

static RParserState* parser_new   (rsexp         context);
static void          parser_parse (RParserState* parser);
static rsexp         parse_datum  (RParserState* parser,
                                   rboolean      expect);

static inline RParserState* parser_new (rsexp context)
{
    RParserState* parser;

    parser = GC_NEW (RParserState);
    memset (parser, 0, sizeof (RParserState));

    parser->context    = context;
    parser->input_port = R_SEXP_UNDEFINED;
    parser->tree       = R_SEXP_UNDEFINED;
    parser->error_type = R_SEXP_UNDEFINED;
    parser->last_error = R_SEXP_UNDEFINED;
    parser->scanner    = r_scanner_new ();

    return parser;
}

static void raise_syntax_error (RParserState* parser,
                                char const*   message)
{
    R_CACHED_SYMBOL (type, "syntax", parser->context);

    parser->error_type = type;
    parser->last_error = r_error (r_string_new (message), R_SEXP_NULL);

    // TODO add location information

    longjmp (parser->jmp, 1);
}

static rtokenid peek_id (RParserState* parser) {
    return r_scanner_peek_id (parser->scanner, parser->input_port);
}

static RToken* next_token (RParserState* parser) {
    return r_scanner_next_token (parser->scanner, parser->input_port);
}

static void consume_token (RParserState* parser) {
    r_scanner_consume_token (parser->scanner, parser->context);
}

static rsexp parse_boolean (RParserState* parser, rboolean expect)
{
    if (TKN_BOOLEAN != peek_id (parser)) {
        if (expect)
            raise_syntax_error (parser, "expecting a boolean");
        else
            return R_SEXP_UNSPECIFIED;
    }

    RToken* t = next_token (parser);
    rsexp res = ('t' == t->text [0]) ? R_SEXP_TRUE : R_SEXP_FALSE;
    r_scanner_free_token (t);

    return res;
}

static rsexp parse_string (RParserState* parser, rboolean expect)
{
    if (TKN_STRING != peek_id (parser)) {
        if (expect)
            raise_syntax_error (parser, "expecting a string");
        else
            return R_SEXP_UNSPECIFIED;
    }

    RToken* t = next_token (parser);
    rsexp tree = r_string_new ((char*) t->text);
    r_scanner_free_token (t);

    return tree;
}

static rsexp parse_symbol (RParserState* parser, rboolean expect)
{
    if (TKN_IDENTIFIER != peek_id (parser)) {
        if (expect)
            raise_syntax_error (parser, "expecting an identifier");
        else
            return R_SEXP_UNSPECIFIED;
    }

    RToken* t = next_token (parser);
    rsexp tree = r_symbol_new ((char*) t->text, parser->context);
    r_scanner_free_token (t);

    return tree;
}

static rsexp parse_simple_datum (RParserState* parser, rboolean expect)
{
    rsexp tree = parse_boolean (parser, FALSE);

    if (r_unspecified_p (tree))
        tree = parse_string (parser, FALSE);

    if (r_unspecified_p (tree))
        tree = parse_symbol (parser, FALSE);

    if (expect && r_unspecified_p (tree))
        raise_syntax_error (parser, "invalid simple datum");

    return tree;
}

static rsexp parse_abbrev (RParserState* parser, rboolean expect)
{
    rsexp res;
    rsexp obj;
    rtokenid id;

    R_CACHED_SYMBOL (q,  "quote",            parser->context);
    R_CACHED_SYMBOL (qq, "quasiquote",       parser->context);
    R_CACHED_SYMBOL (u,  "unquote",          parser->context);
    R_CACHED_SYMBOL (us, "unquote-splicing", parser->context);

    id  = peek_id (parser);
    res = R_SEXP_UNSPECIFIED;

    switch (id) {
        case TKN_QUOTE:    res = q;  break;
        case TKN_BACKTICK: res = qq; break;
        case TKN_COMMA:    res = u;  break;
        case TKN_COMMA_AT: res = us; break;
    }

    if (r_unspecified_p (res)) {
        if (expect)
            raise_syntax_error (parser, "expecting an abbreviated list");
        else
            return res;
    }

    consume_token (parser);
    obj = parse_datum (parser, TRUE);
    res = r_list (2, res, obj);

    return res;
}

static rsexp parse_list (RParserState* parser, rboolean expect)
{
    rsexp res;
    rsexp obj;
    rsexp last;

    // Handle abbreviations (quote, unquote, unquote-splicing & quasiquote).
    res = parse_abbrev (parser, FALSE);

    if (!r_unspecified_p (res))
        return res;

    // Consume the ` ('.
    if (TKN_LP == peek_id (parser))
        consume_token (parser);
    else if (expect)
        raise_syntax_error (parser, "expecting `('");
    else
        return R_SEXP_UNSPECIFIED;

    // Initialize the list to '().  Read data until `.' or `)', cons the list in
    // reverse order and then revert it.
    for (res = R_SEXP_NULL;
         !r_unspecified_p (obj = parse_datum (parser, FALSE));
         res = r_cons (obj, res))
        ;

    res = r_reverse (res);

    // Handle dotted improper list like (a b c . d).
    if (TKN_DOT == peek_id (parser)) {
        consume_token (parser);
        last = parse_datum (parser, TRUE);
        r_append_x (res, last);
    }

    // Consume the `)'.
    if (TKN_RP == peek_id (parser))
        consume_token (parser);
    else
        raise_syntax_error (parser, "expecting `)'");

    return res;
}

static rsexp parse_vector (RParserState* parser, rboolean expect)
{
    rsexp acc;
    rsexp obj;

    // Consume the `#('.
    if (TKN_HASH_LP == peek_id (parser))
        consume_token (parser);
    else if (expect)
        raise_syntax_error (parser, "expecting `#('");
    else
        return R_SEXP_UNSPECIFIED;

    // Read vector element into a list.
    for (acc = R_SEXP_NULL;
         !r_unspecified_p (obj = parse_datum (parser, FALSE));
         acc = r_cons (obj, acc))
        ;

    // Consume the `)'.
    if (TKN_RP == peek_id (parser))
        consume_token (parser);
    else
        raise_syntax_error (parser, "expecting `)'");

    // Convert the list to a vector.
    return r_list_to_vector (r_reverse (acc));
}

static rsexp parse_compound_datum (RParserState* parser, rboolean expect)
{
    rsexp tree = parse_list (parser, FALSE);

    if (r_unspecified_p (tree))
        tree = parse_vector (parser, FALSE);

    if (expect && r_unspecified_p (tree))
        raise_syntax_error (parser, "invalid compound datum");

    return tree;
}

static rsexp parse_datum (RParserState* parser, rboolean expect)
{
    rsexp tree = parse_simple_datum (parser, FALSE);

    if (r_unspecified_p (tree))
        tree = parse_compound_datum (parser, FALSE);

    if (expect && r_unspecified_p (tree))
        raise_syntax_error (parser, "invalid datum");

    return tree;
}

static rsexp parse_data (RParserState* parser, rboolean expect)
{
    rsexp tree = R_SEXP_NULL;
    rsexp datum = R_SEXP_NULL;

    while (TKN_EOF != peek_id (parser)) {
        datum = parse_datum (parser, TRUE);
        tree = r_cons (datum, tree);
    }

    return r_reverse (tree);
}

static inline void parser_parse (RParserState* parser)
{
    if (setjmp (parser->jmp)) {
        return;
    }

    rsexp tree = parse_data (parser, TRUE);
    parser->tree = tree;
}

rsexp r_parser_result (RParserState* parser)
{
    return parser->tree;
}

RParserState* r_parse_file (char* filename, rsexp context)
{
    RParserState* parser = parser_new (context);
    parser->input_port = r_open_input_file (filename);
    parser_parse (parser);
    return parser;
}

RParserState* r_parse_string (char* string, rsexp context)
{
    RParserState* parser = parser_new (context);
    parser->input_port = r_open_input_string (string);
    parser_parse (parser);
    return parser;
}

RParserState* r_parse_port (rsexp port, rsexp context)
{
    RParserState* parser = parser_new (context);
    parser->input_port = port;
    parser_parse (parser);
    return parser;
}

rsexp r_parser_error (RParserState* parser)
{
    return parser->error_type;
}

rsexp r_parser_last_error (RParserState* parser)
{
    return parser->last_error;
}
