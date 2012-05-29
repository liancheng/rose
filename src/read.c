#include "scanner.h"

#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/read.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"

#include <gc/gc.h>
#include <setjmp.h>

struct RReaderState {
    rsexp     context;
    rsexp     input_port;
    rsexp     tree;
    rsexp     error_type;
    rsexp     last_error;
    RScanner* scanner;
    jmp_buf   jmp;
};

static RReaderState* reader_new   (rsexp         context);
static void          reader_parse (RReaderState* reader);
static rsexp         read_datum   (RReaderState* reader,
                                   rboolean      expect);

static inline RReaderState* reader_new (rsexp context)
{
    RReaderState* reader;

    reader = GC_NEW (RReaderState);
    memset (reader, 0, sizeof (RReaderState));

    reader->context    = context;
    reader->input_port = R_SEXP_UNDEFINED;
    reader->tree       = R_SEXP_UNDEFINED;
    reader->error_type = R_SEXP_UNDEFINED;
    reader->last_error = R_SEXP_UNDEFINED;
    reader->scanner    = r_scanner_new ();

    return reader;
}

static void raise_syntax_error (RReaderState* reader,
                                char const*   message)
{
    R_CACHED_SYMBOL (type, "syntax", reader->context);

    rsexp error = r_error (r_string_new (message),
                           r_cons (r_int_to_sexp (r_reader_line (reader)),
                                   r_int_to_sexp (r_reader_column (reader))));

    reader->error_type = type;
    reader->last_error = error;

    longjmp (reader->jmp, 1);
}

static rtokenid peek_id (RReaderState* reader) {
    return r_scanner_peek_id (reader->scanner, reader->input_port);
}

static RToken* next_token (RReaderState* reader) {
    return r_scanner_next_token (reader->scanner, reader->input_port);
}

static void consume_token (RReaderState* reader) {
    r_scanner_consume_token (reader->scanner, reader->context);
}

static rsexp read_boolean (RReaderState* reader, rboolean expect)
{
    if (TKN_BOOLEAN != peek_id (reader)) {
        if (expect)
            raise_syntax_error (reader, "expecting a boolean");
        else
            return R_SEXP_UNSPECIFIED;
    }

    RToken* t = next_token (reader);
    rsexp res = ('t' == t->text [0]) ? R_SEXP_TRUE : R_SEXP_FALSE;
    r_scanner_free_token (t);

    return res;
}

static rsexp read_string (RReaderState* reader, rboolean expect)
{
    if (TKN_STRING != peek_id (reader)) {
        if (expect)
            raise_syntax_error (reader, "expecting a string");
        else
            return R_SEXP_UNSPECIFIED;
    }

    RToken* t = next_token (reader);
    rsexp tree = r_string_new ((char*) t->text);
    r_scanner_free_token (t);

    return tree;
}

static rsexp read_symbol (RReaderState* reader, rboolean expect)
{
    if (TKN_IDENTIFIER != peek_id (reader)) {
        if (expect)
            raise_syntax_error (reader, "expecting an identifier");
        else
            return R_SEXP_UNSPECIFIED;
    }

    RToken* t = next_token (reader);
    rsexp tree = r_symbol_new ((char*) t->text, reader->context);
    r_scanner_free_token (t);

    return tree;
}

static rsexp read_simple_datum (RReaderState* reader, rboolean expect)
{
    rsexp tree = read_boolean (reader, FALSE);

    if (r_unspecified_p (tree))
        tree = read_string (reader, FALSE);

    if (r_unspecified_p (tree))
        tree = read_symbol (reader, FALSE);

    if (expect && r_unspecified_p (tree))
        raise_syntax_error (reader, "invalid simple datum");

    return tree;
}

static rsexp read_abbrev (RReaderState* reader, rboolean expect)
{
    rsexp res;
    rsexp obj;
    rtokenid id;

    R_CACHED_SYMBOL (q,  "quote",            reader->context);
    R_CACHED_SYMBOL (qq, "quasiquote",       reader->context);
    R_CACHED_SYMBOL (u,  "unquote",          reader->context);
    R_CACHED_SYMBOL (us, "unquote-splicing", reader->context);

    id  = peek_id (reader);
    res = R_SEXP_UNSPECIFIED;

    switch (id) {
        case TKN_QUOTE:    res = q;  break;
        case TKN_BACKTICK: res = qq; break;
        case TKN_COMMA:    res = u;  break;
        case TKN_COMMA_AT: res = us; break;
    }

    if (r_unspecified_p (res)) {
        if (expect)
            raise_syntax_error (reader, "expecting an abbreviated list");
        else
            return res;
    }

    consume_token (reader);
    obj = read_datum (reader, TRUE);
    res = r_list (2, res, obj);

    return res;
}

static rsexp read_list (RReaderState* reader, rboolean expect)
{
    rsexp res;
    rsexp obj;
    rsexp last;

    // Handle abbreviations (quote, unquote, unquote-splicing & quasiquote).
    res = read_abbrev (reader, FALSE);

    if (!r_unspecified_p (res))
        return res;

    // Consume the ` ('.
    if (TKN_LP == peek_id (reader))
        consume_token (reader);
    else if (expect)
        raise_syntax_error (reader, "expecting `('");
    else
        return R_SEXP_UNSPECIFIED;

    // Initialize the list to '().  Read data until `.' or `)', cons the list in
    // reverse order and then revert it.
    for (res = R_SEXP_NULL;
         !r_unspecified_p (obj = read_datum (reader, FALSE));
         res = r_cons (obj, res))
        ;

    res = r_reverse (res);

    // Handle dotted improper list like (a b c . d).
    if (TKN_DOT == peek_id (reader)) {
        consume_token (reader);
        last = read_datum (reader, TRUE);
        r_append_x (res, last);
    }

    // Consume the `)'.
    if (TKN_RP == peek_id (reader))
        consume_token (reader);
    else
        raise_syntax_error (reader, "expecting `)'");

    return res;
}

static rsexp read_vector (RReaderState* reader, rboolean expect)
{
    rsexp acc;
    rsexp obj;

    // Consume the `#('.
    if (TKN_HASH_LP == peek_id (reader))
        consume_token (reader);
    else if (expect)
        raise_syntax_error (reader, "expecting `#('");
    else
        return R_SEXP_UNSPECIFIED;

    // Read vector element into a list.
    for (acc = R_SEXP_NULL;
         !r_unspecified_p (obj = read_datum (reader, FALSE));
         acc = r_cons (obj, acc))
        ;

    // Consume the `)'.
    if (TKN_RP == peek_id (reader))
        consume_token (reader);
    else
        raise_syntax_error (reader, "expecting `)'");

    // Convert the list to a vector.
    return r_list_to_vector (r_reverse (acc));
}

static rsexp read_compound_datum (RReaderState* reader, rboolean expect)
{
    rsexp tree = read_list (reader, FALSE);

    if (r_unspecified_p (tree))
        tree = read_vector (reader, FALSE);

    if (expect && r_unspecified_p (tree))
        raise_syntax_error (reader, "invalid compound datum");

    return tree;
}

static rsexp read_datum (RReaderState* reader, rboolean expect)
{
    rsexp tree = read_simple_datum (reader, FALSE);

    if (r_unspecified_p (tree))
        tree = read_compound_datum (reader, FALSE);

    if (expect && r_unspecified_p (tree))
        raise_syntax_error (reader, "invalid datum");

    return tree;
}

static rsexp read_data (RReaderState* reader, rboolean expect)
{
    rsexp tree = R_SEXP_NULL;
    rsexp datum = R_SEXP_NULL;

    while (TKN_EOF != peek_id (reader)) {
        datum = read_datum (reader, TRUE);
        tree = r_cons (datum, tree);
    }

    return r_reverse (tree);
}

static inline void reader_parse (RReaderState* reader)
{
    if (setjmp (reader->jmp)) {
        return;
    }

    rsexp tree = read_data (reader, TRUE);
    reader->tree = tree;
}

rsexp r_reader_result (RReaderState* reader)
{
    return reader->tree;
}

RReaderState* r_read_from_file (char* filename, rsexp context)
{
    RReaderState* reader = reader_new (context);
    reader->input_port = r_open_input_file (filename);
    reader_parse (reader);
    return reader;
}

RReaderState* r_read_from_string (char* string, rsexp context)
{
    RReaderState* reader = reader_new (context);
    reader->input_port = r_open_input_string (string);
    reader_parse (reader);
    return reader;
}

RReaderState* r_read_from_port (rsexp port, rsexp context)
{
    RReaderState* reader = reader_new (context);
    reader->input_port = port;
    reader_parse (reader);
    return reader;
}

rsexp r_reader_error (RReaderState* reader)
{
    return reader->error_type;
}

rsexp r_reader_last_error (RReaderState* reader)
{
    return reader->last_error;
}

int r_reader_line (RReaderState* reader)
{
    return r_scanner_line (reader->scanner);
}

int r_reader_column (RReaderState* reader)
{
    return r_scanner_column (reader->scanner);
}
