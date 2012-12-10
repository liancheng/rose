#include "detail/reader.h"
#include "detail/state.h"
#include "rose/bytevector.h"
#include "rose/error.h"
#include "rose/memory.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/vector.h"

#include <alloca.h>
#include <assert.h>
#include <stdarg.h>

static rsexp read_datum (RDatumReader* reader);

static rint feed_lexer (RDatumReader* reader)
{
    QUEX_NAME (buffer_fill_region_prepare) (&reader->lexer);

    rcstring begin = r_cast (rcstring,
                             QUEX_NAME (buffer_fill_region_begin)
                                       (&reader->lexer));

    RState*  state = reader->state;
    rint     size  = QUEX_NAME (buffer_fill_region_size) (&reader->lexer);
    rcstring line  = r_port_gets (state, reader->input_port, begin, size);
    rint     len   = line ? strlen (line) : 0;

    QUEX_NAME (buffer_fill_region_finish) (&reader->lexer, len);

    return len;
}

static RToken* next_token (RDatumReader* reader)
{
    rtokenid id;

    do
        id = QUEX_NAME (receive) (&reader->lexer);
    while (TKN_TERMINATION == id && feed_lexer (reader));

    return QUEX_NAME (token_p) (&reader->lexer);
}

static void consume (RDatumReader* reader)
{
    if (reader->lookahead)
        reader->lookahead = NULL;
}

static RToken* lookahead (RDatumReader* reader)
{
    if (!reader->lookahead)
        reader->lookahead = next_token (reader);

    return reader->lookahead;
}

static rbool match (RDatumReader* reader, rtokenid id)
{
    if (id == lookahead (reader)->_id) {
        consume (reader);
        return TRUE;
    }

    return FALSE;
}

static rsexp lexeme_to_char (char const* text)
{
    char res = *text;

    if ('\0' != text [1]) {
        if      (0 == strncmp (text, "space",      6u)) res = ' ';
        else if (0 == strncmp (text, "tab",        4u)) res = '\t';
        else if (0 == strncmp (text, "newline",    8u)) res = '\n';
        else if (0 == strncmp (text, "return",     7u)) res = '\r';
        else if (0 == strncmp (text, "null",       5u)) res = '\0';
        else if (0 == strncmp (text, "alarm",      6u)) res = '\a';
        else if (0 == strncmp (text, "backspace", 10u)) res = '\b';
        else if (0 == strncmp (text, "escape",     7u)) res = '\x1b';
        else if (0 == strncmp (text, "delete",     7u)) res = '\x7f';
    }

    return r_char_to_sexp (res);
}

static void syntax_error (RDatumReader* reader,
                          rsize         line,
                          rsize         column,
                          char const*   message)
{
    rsexp str = r_string_format (reader->state,
                                 "~a:~a:~a: ~a~%",
                                 r_port_get_name (reader->input_port),
                                 r_int_to_sexp (line),
                                 r_int_to_sexp (column),
                                 r_string_new (reader->state, message));

    r_raise (reader->state, r_error_new (reader->state, str, R_NULL));
}

static void record_source_location (RDatumReader* reader,
                                    rsize*        line,
                                    rsize*        column)
{
    *line   = lookahead (reader)->_line_n;
    *column = lookahead (reader)->_column_n;
}

static rsexp read_vector (RDatumReader* reader)
{
    rsexp datum;
    rsexp list;
    rsize line;
    rsize column;

    if (!match (reader, TKN_HASH_LP))
        return R_ERROR_BACKTRACK;

    for (list = R_NULL; lookahead (reader)->_id != TKN_RP; ) {
        record_source_location (reader, &line, &column);

        datum = read_datum (reader);

        if (r_eof_object_p (datum)) {
            rcstring message = "the vector is not closed";
            syntax_error (reader, line, column, message);
        }

        if (r_error_p (datum)) {
            rcstring message = "expecting a vector element";
            syntax_error (reader, line, column, message);
        }

        list = r_cons (reader->state, datum, list);
    }

    consume (reader);

    return r_list_to_vector (reader->state, r_reverse (list));
}

static rsexp read_full_list (RDatumReader* reader)
{
    rsexp list;
    rsexp datum;
    rsize line;
    rsize column;

    if (!match (reader, TKN_LP))
        return R_ERROR_BACKTRACK;

    if (match (reader, TKN_RP))
        return R_NULL;

    record_source_location (reader, &line, &column);

    datum = read_datum (reader);
    if (r_error_p (datum))
        syntax_error (reader, line, column, "bad syntax");

    list = r_cons (reader->state, datum, R_NULL);

    while (TRUE) {
        rtokenid id = lookahead (reader)->_id;

        if (id == TKN_DOT || id == TKN_RP)
            break;

        record_source_location (reader, &line, &column);

        datum = read_datum (reader);
        if (r_error_p (datum))
            syntax_error (reader, line, column, "bad syntax");

        list = r_cons (reader->state, datum, list);
    }

    list = r_reverse (list);

    if (match (reader, TKN_DOT)) {
        record_source_location (reader, &line, &column);

        datum = read_datum (reader);
        if (r_error_p (datum))
            syntax_error (reader, line, column, "datum expected");

        list = r_append_x (list, datum);
    }

    record_source_location (reader, &line, &column);

    if (!match (reader, TKN_RP))
        syntax_error (reader, line, column,
                      "missing close parenthesis");

    return list;
}

static rsexp read_abbreviation (RDatumReader* reader)
{
    rsexp prefix;
    rsexp datum;
    rsize line;
    rsize column;

    switch (lookahead (reader)->_id) {
        case TKN_QUOTE:
            prefix = keyword (reader->state, R_QUOTE);
            break;

        case TKN_BACKTICK:
            prefix = keyword (reader->state, R_QUASIQUOTE);
            break;

        case TKN_COMMA:
            prefix = keyword (reader->state, R_UNQUOTE);
            break;

        case TKN_COMMA_AT:
            prefix = keyword (reader->state, R_UNQUOTE_SPLICING);
            break;

        default:
            return R_ERROR_BACKTRACK;
    }

    consume (reader);
    record_source_location (reader, &line, &column);

    datum = read_datum (reader);
    if (r_error_p (datum))
        syntax_error (reader, line, column, "bad syntax");

    return r_list (reader->state, 2, prefix, datum);
}

static rsexp read_list (RDatumReader* reader)
{
    return TKN_LP == lookahead (reader)->_id
           ? read_full_list (reader)
           : read_abbreviation (reader);
}

static rsexp read_compound_datum (RDatumReader* reader)
{
    return TKN_HASH_LP == lookahead (reader)->_id
           ? read_vector (reader)
           : read_list (reader);
}

static rsexp read_bytevector (RDatumReader* reader)
{
    rsexp bytes;
    rsexp datum;
    rsize line;
    rsize column;

    if (!match (reader, TKN_HASH_U8_LP))
        return R_ERROR_BACKTRACK;

    bytes = R_NULL;

    while (lookahead (reader)->_id != TKN_RP) {
        record_source_location (reader, &line, &column);

        datum = read_datum (reader);

        if (!r_byte_p (datum))
            syntax_error (reader, line, column, "value out of range");

        bytes = r_cons (reader->state, datum, bytes);
    }

    if (!match (reader, TKN_RP))
        return R_ERROR_BACKTRACK;

    return r_list_to_bytevector (reader->state, r_reverse (bytes));
}

static rsexp read_simple_datum (RDatumReader* reader)
{
    RToken*  token;
    rsexp    datum;
    rcstring text;
    rsize    size;

    token = lookahead (reader);
    size  = QUEX_NAME (strlen) (token->text) + 5;
    text  = alloca (size);

    QUEX_NAME_TOKEN (pretty_char_text) (token, text, size);

    switch (token->_id) {
        case TKN_TRUE:
            datum = R_TRUE;
            break;

        case TKN_FALSE:
            datum = R_FALSE;
            break;

        case TKN_NUMBER:
            datum = r_string_to_number (reader->state, text);
            break;

        case TKN_CHARACTER:
            datum = lexeme_to_char (text);
            break;

        case TKN_STRING:
            datum = r_string_new (reader->state, text);
            break;

        case TKN_IDENTIFIER:
            datum = r_symbol_new (reader->state, text);
            break;

        case TKN_HASH_U8_LP:
            datum = read_bytevector (reader);
            break;

        default:
            return R_ERROR_BACKTRACK;
    }

    consume (reader);

    return datum;
}

static rsexp read_datum (RDatumReader* reader)
{
    rsexp datum;

    if (match (reader, TKN_HASH_SEMICOLON))
        if (r_error_p (read_datum (reader)))
            return R_ERROR_BACKTRACK;

    datum = read_simple_datum (reader);

    return r_error_p (datum) ? read_compound_datum (reader) : datum;
}

static void reader_init (RState* state, RDatumReader* reader, rsexp port)
{
    memset (reader, 0, sizeof (RDatumReader));

    reader->state      = state;
    reader->input_port = port;
    reader->lookahead  = NULL;

    QUEX_NAME (construct_memory) (&reader->lexer, NULL, 0, NULL, NULL, FALSE);
    QUEX_NAME_TOKEN (construct) (&reader->token);
    QUEX_NAME (token_p_set) (&reader->lexer, &reader->token);
}

static void reader_destruct (RDatumReader* reader)
{
    QUEX_NAME (destruct) (&reader->lexer);
    QUEX_NAME_TOKEN (destruct) (&reader->token);
}

rsexp r_port_read (RState* state, rsexp port)
{
    RDatumReader reader;
    RNestedJump  jmp;
    rsexp        datum;

    reader_init (state, &reader, port);

    r_try (jmp, state) {
        if (lookahead (&reader)->_id == TKN_TERMINATION)
            return R_EOF;

        datum = read_datum (&reader);
        goto clean;
    }
    r_catch {
        datum = r_last_error (state);
        goto clean;
    }
    r_end_try (state);

clean:
    reader_destruct (&reader);

    return datum;
}

rsexp r_read (RState* state)
{
    return r_port_read (state, r_current_input_port (state));
}
