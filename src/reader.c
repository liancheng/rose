#include "detail/raise.h"
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

static rsexp read_datum (RState* state, RDatumReader* reader);

static rint feed_lexer (RDatumReader* reader)
{
    QUEX_NAME (buffer_fill_region_prepare) (reader->lexer);

    rcstring begin = r_cast (rcstring,
                             QUEX_NAME (buffer_fill_region_begin)
                                       (reader->lexer));

    rint     size = QUEX_NAME (buffer_fill_region_size) (reader->lexer);
    rcstring line = r_port_gets (reader->input_port, begin, size);
    rint     len  = line ? strlen (line) : 0;

    QUEX_NAME (buffer_fill_region_finish) (reader->lexer, len);

    return len;
}

static RToken* next_token (RDatumReader* reader)
{
    rtokenid id;

    do
        id = QUEX_NAME (receive) (reader->lexer);
    while (TKN_TERMINATION == id && feed_lexer (reader));

    return QUEX_NAME (token_p) (reader->lexer);
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

static RLexer* lexer_new (RState* state)
{
    RLexer* lexer = r_new (state, RLexer);
    QUEX_NAME (construct_memory) (lexer, NULL, 0, NULL, NULL, FALSE);

    return lexer;
}

static void lexer_free (RState* state, RLexer* lexer)
{
    QUEX_NAME (destruct) (lexer);
    r_free (state, lexer);
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

static void syntax_error (RState*       state,
                          RDatumReader* reader,
                          rsize         line,
                          rsize         column,
                          char const*   message)
{
    rsexp e = r_error_new (state,
                           r_string_new (state, message),
                           r_list (state,
                                   3,
                                   r_port_get_name (reader->input_port),
                                   r_int_to_sexp (line),
                                   r_int_to_sexp (column)));

    r_raise (state, e);
}

static void record_source_location (RDatumReader* reader,
                                    rsize*        line,
                                    rsize*        column)
{
    *line   = lookahead (reader)->_line_n;
    *column = lookahead (reader)->_column_n;
}

static rsexp read_vector (RState* state, RDatumReader* reader)
{
    rsexp datum;
    rsexp list;
    rsize line;
    rsize column;

    if (!match (reader, TKN_HASH_LP))
        return R_UNSPECIFIED;

    for (list = R_NULL; lookahead (reader)->_id != TKN_RP; ) {
        record_source_location (reader, &line, &column);

        datum = read_datum (state, reader);

        if (r_eof_object_p (datum)) {
            rcstring message = "the vector is not closed";
            syntax_error (state, reader, line, column, message);
        }

        if (r_unspecified_p (datum)) {
            rcstring message = "expecting a vector element";
            syntax_error (state, reader, line, column, message);
        }

        list = r_cons (state, datum, list);
    }

    consume (reader);

    return r_list_to_vector (state, r_reverse (list));
}

static rsexp read_full_list (RState* state, RDatumReader* reader)
{
    rsexp list;
    rsexp datum;
    rsize line;
    rsize column;

    if (!match (reader, TKN_LP))
        return R_UNSPECIFIED;

    if (match (reader, TKN_RP))
        return R_NULL;

    record_source_location (reader, &line, &column);

    datum = read_datum (state, reader);
    if (r_unspecified_p (datum))
        syntax_error (state, reader, line, column, "bad syntax");

    list = r_cons (state, datum, R_NULL);

    while (TRUE) {
        rtokenid id = lookahead (reader)->_id;

        if (id == TKN_DOT || id == TKN_RP)
            break;

        record_source_location (reader, &line, &column);

        datum = read_datum (state, reader);
        if (r_unspecified_p (datum))
            syntax_error (state, reader, line, column, "bad syntax");

        list = r_cons (state, datum, list);
    }

    list = r_reverse (list);

    if (match (reader, TKN_DOT)) {
        record_source_location (reader, &line, &column);

        datum = read_datum (state, reader);
        if (r_unspecified_p (datum))
            syntax_error (state, reader, line, column, "datum expected");

        list = r_append_x (list, datum);
    }

    record_source_location (reader, &line, &column);

    if (!match (reader, TKN_RP))
        syntax_error (state, reader, line, column,
                      "missing close parenthesis");

    return list;
}

static rsexp read_abbreviation (RState* state, RDatumReader* reader)
{
    rsexp prefix;
    rsexp datum;
    rsize line;
    rsize column;

    switch (lookahead (reader)->_id) {
        case TKN_QUOTE:
            prefix = keyword (state, R_QUOTE);
            break;

        case TKN_BACKTICK:
            prefix = keyword (state, R_QUASIQUOTE);
            break;

        case TKN_COMMA:
            prefix = keyword (state, R_UNQUOTE);
            break;

        case TKN_COMMA_AT:
            prefix = keyword (state, R_UNQUOTE_SPLICING);
            break;

        default:
            return R_UNSPECIFIED;
    }

    consume (reader);
    record_source_location (reader, &line, &column);

    datum = read_datum (state, reader);
    if (r_unspecified_p (datum))
        syntax_error (state, reader, line, column, "bad syntax");

    return r_list (state, 2, prefix, datum);
}

static rsexp read_list (RState* state, RDatumReader* reader)
{
    return TKN_LP == lookahead (reader)->_id
           ? read_full_list (state, reader)
           : read_abbreviation (state, reader);
}

static rsexp read_compound_datum (RState* state, RDatumReader* reader)
{
    return TKN_HASH_LP == lookahead (reader)->_id
           ? read_vector (state, reader)
           : read_list (state, reader);
}

static rsexp read_bytevector (RState* state, RDatumReader* reader)
{
    rsexp bytes;
    rsexp datum;
    rsize line;
    rsize column;

    if (!match (reader, TKN_HASH_U8_LP))
        return R_UNSPECIFIED;

    bytes = R_NULL;

    while (lookahead (reader)->_id != TKN_RP) {
        record_source_location (reader, &line, &column);

        datum = read_datum (state, reader);

        if (!r_byte_p (datum))
            syntax_error (state, reader, line, column, "value out of range");

        bytes = r_cons (state, datum, bytes);
    }

    if (!match (reader, TKN_RP))
        return R_UNSPECIFIED;

    return r_list_to_bytevector (state, r_reverse (bytes));
}

static rsexp read_simple_datum (RState* state, RDatumReader* reader)
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
            datum = r_string_to_number (state, text);
            break;

        case TKN_CHARACTER:
            datum = lexeme_to_char (text);
            break;

        case TKN_STRING:
            datum = r_string_new (state, text);
            break;

        case TKN_IDENTIFIER:
            datum = r_symbol_new (state, text);
            break;

        case TKN_HASH_U8_LP:
            datum = read_bytevector (state, reader);
            break;

        default:
            return R_UNSPECIFIED;
    }

    consume (reader);

    return datum;
}

static rsexp read_datum (RState* state, RDatumReader* reader)
{
    rsexp datum;

    if (match (reader, TKN_HASH_SEMICOLON))
        if (r_unspecified_p (read_datum (state, reader)))
            return R_UNSPECIFIED;

    datum = read_simple_datum (state, reader);

    return r_unspecified_p (datum)
           ? read_compound_datum (state, reader)
           : datum;
}

rsexp r_read (RState* state, RDatumReader* reader)
{
    rsexp datum;
    RNestedJump jmp;

    R_TRY (jmp, state) {
        if (lookahead (reader)->_id == TKN_TERMINATION)
            return R_EOF;

        datum = read_datum (state, reader);
        return r_unspecified_p (datum) ? R_EOF : datum;
    }
    R_CATCH {
        return R_UNSPECIFIED;
    }
    R_END_TRY (state);
}

RDatumReader* r_reader_new (RState* state)
{
    RDatumReader* reader = r_new0 (state, RDatumReader);

    reader->input_port = r_current_input_port (state);
    reader->last_error = R_UNDEFINED;
    reader->lexer      = lexer_new (state);
    reader->lookahead  = NULL;

    QUEX_NAME (token_p_set) (reader->lexer, &reader->token);

    return reader;
}

void r_reader_free (RState* state, RDatumReader* reader)
{
    lexer_free (state, reader->lexer);
}

RDatumReader* r_file_reader (RState* state, char const* filename)
{
    return r_port_reader (state, r_open_input_file (state, filename));
}

RDatumReader* r_string_reader (RState* state, char const* string)
{
    rsexp input = r_string_new (state, string);
    rsexp port  = r_open_input_string (state, input);
    return r_port_reader (state, port);
}

RDatumReader* r_port_reader (RState* state, rsexp port)
{
    RDatumReader* reader = r_reader_new (state);
    reader->input_port = port;
    return reader;
}

rsexp r_reader_last_error (RState* state, RDatumReader* reader)
{
    return reader->last_error;
}

void r_reader_clear_error (RState* state, RDatumReader* reader)
{
    reader->last_error = R_UNDEFINED;
}
