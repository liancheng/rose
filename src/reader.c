#include "detail/state.h"
#include "detail/reader.h"
#include "rose/bytevector.h"
#include "rose/error.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/vector.h"

#include <alloca.h>
#include <gc/gc.h>
#include <stdarg.h>

#define MATCH_OR_RETURN_UNSPECIFIED(id)\
        do {\
            if (lookahead (reader)->_id != id)\
                return R_UNSPECIFIED;\
        }\
        while (0)

#define SOURCE_LOCATION(line, column)\
        do {\
            (line)   = lookahead (reader)->_line_n;\
            (column) = lookahead (reader)->_column_n;\
        }\
        while (0)

static rsexp read_datum (RDatumReader* reader);

static rint reload_lexer (RDatumReader* reader)
{
    QUEX_NAME (buffer_fill_region_prepare) (reader->lexer);

    char* begin = (char*) QUEX_NAME (buffer_fill_region_begin) (reader->lexer);
    rint  size  = QUEX_NAME (buffer_fill_region_size) (reader->lexer);
    char* line  = r_port_gets (reader->input_port, begin, size);
    rint  len   = line ? strlen (line) : 0;

    QUEX_NAME (buffer_fill_region_finish) (reader->lexer, len);

    return len;
}

static RToken* next_token (RDatumReader* reader)
{
    rtokenid id;

    do
        id = QUEX_NAME (receive) (reader->lexer);
    while (TKN_TERMINATION == id && reload_lexer (reader));

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

static void lexer_finalize (rpointer obj, rpointer client_data)
{
    QUEX_NAME (destruct) ((RLexer*) obj);
}

static RLexer* lexer_new ()
{
    RLexer* lexer = GC_NEW_ATOMIC (RLexer);

    QUEX_NAME (construct_memory) (lexer, NULL, 0, NULL, NULL, FALSE);
    GC_REGISTER_FINALIZER (lexer, lexer_finalize, NULL, NULL, NULL);

    return lexer;
}

static rsexp lexeme_to_char (char const* text)
{
    rint len = strlen (text);
    char res = *text;

    if (len > 1) {
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

static void raise_reader_error (RDatumReader* reader,
                                rsize         line,
                                rsize         column,
                                char const*   message)
{
    reader->last_error =
        r_error_new (r_string_new (message),
                     r_list (3,
                             r_port_get_name (reader->input_port),
                             r_int_to_sexp (line),
                             r_int_to_sexp (column)));

    longjmp (reader->jmp, 1);
}

void r_reader_clear_error (RDatumReader* reader)
{
    reader->last_error = R_UNDEFINED;
}

static rsexp read_vector (RDatumReader* reader)
{
    rsexp datum;
    rsexp list;
    rsize line;
    rsize column;

    MATCH_OR_RETURN_UNSPECIFIED (TKN_HASH_LP);

    consume (reader);

    SOURCE_LOCATION (line, column);

    for (list = R_NULL; lookahead (reader)->_id != TKN_RP; ) {
        datum = read_datum (reader);

        if (r_unspecified_p (datum))
            raise_reader_error (reader, line, column, "bad syntax");

        list = r_cons (datum, list);
    }

    consume (reader);

    return r_list_to_vector (r_reverse (list));
}

static rsexp read_full_list (RDatumReader* reader)
{
    rsexp list;
    rsexp datum;
    rsize line;
    rsize column;

    MATCH_OR_RETURN_UNSPECIFIED (TKN_LP);
    consume (reader);

    if (lookahead (reader)->_id == TKN_RP) {
        consume (reader);
        return R_NULL;
    }

    SOURCE_LOCATION (line, column);

    datum = read_datum (reader);

    if (r_unspecified_p (datum))
        raise_reader_error (reader, line, column, "bad syntax");

    list = r_cons (datum, R_NULL);

    while (1) {
        rtokenid id = lookahead (reader)->_id;

        if (id == TKN_DOT || id == TKN_RP)
            break;

        SOURCE_LOCATION (line, column);

        datum = read_datum (reader);

        if (r_unspecified_p (datum))
            raise_reader_error (reader, line, column, "bad syntax");

        list = r_cons (datum, list);
    }

    list = r_reverse (list);

    if (lookahead (reader)->_id == TKN_DOT) {
        consume (reader);
        SOURCE_LOCATION (line, column);

        datum = read_datum (reader);

        if (r_unspecified_p (datum))
            raise_reader_error (reader, line, column, "missing close parenthesis");

        list = r_append_x (list, datum);
    }

    SOURCE_LOCATION (line, column);

    if (lookahead (reader)->_id != TKN_RP)
        raise_reader_error (reader, line, column, "bad syntax");

    consume (reader);

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
            prefix = r_keyword (reader->state, R_QUOTE);
            break;

        case TKN_BACKTICK:
            prefix = r_keyword (reader->state, R_QUASIQUOTE);
            break;

        case TKN_COMMA:
            prefix = r_keyword (reader->state, R_UNQUOTE);
            break;

        case TKN_COMMA_AT:
            prefix = r_keyword (reader->state, R_UNQUOTE_SPLICING);
            break;

        default:
            return R_UNSPECIFIED;
    }

    consume (reader);
    SOURCE_LOCATION (line, column);

    datum = read_datum (reader);

    if (r_unspecified_p (datum))
        raise_reader_error (reader, line, column, "bad syntax");

    return r_list (2, prefix, datum);
}

static rsexp read_list (RDatumReader* reader)
{
    rsexp obj = read_abbreviation (reader);

    return r_unspecified_p (obj) ? read_full_list (reader) : obj;
}

static rsexp read_compound_datum (RDatumReader* reader)
{
    rsexp obj = read_list (reader);

    return r_unspecified_p (obj) ? read_vector (reader) : obj;
}

static rsexp read_bytevector (RDatumReader* reader)
{
    rsexp bytes;
    rsexp datum;
    rsize line;
    rsize column;

    MATCH_OR_RETURN_UNSPECIFIED (TKN_HASH_U8_LP);
    consume (reader);

    bytes = R_NULL;

    while (lookahead (reader)->_id != TKN_RP) {
        SOURCE_LOCATION (line, column);

        datum = read_datum (reader);

        if (!r_byte_p (datum))
            raise_reader_error (reader, line, column, "value out of range");

        bytes = r_cons (datum, bytes);
    }

    MATCH_OR_RETURN_UNSPECIFIED (TKN_RP);
    consume (reader);

    return r_list_to_bytevector (r_reverse (bytes));
}

static rsexp read_simple_datum (RDatumReader* reader)
{
    RToken* token;
    rsexp   datum;
    char*   text;
    rsize   size;

    token = lookahead (reader);
    size  = QUEX_NAME (strlen) (token->text) + 5;
    text  = alloca (size);

    QUEX_NAME_TOKEN (pretty_char_text) (token, text, size);

    switch (token->_id) {
        case TKN_BOOLEAN:
            datum = (token->text [0] == 't') ? R_TRUE : R_FALSE;
            break;

        case TKN_NUMBER:
            datum = r_string_to_number (text);
            break;

        case TKN_CHARACTER:
            datum = lexeme_to_char (text);
            break;

        case TKN_STRING:
            datum = r_string_new (text);
            break;

        case TKN_IDENTIFIER:
            datum = r_symbol_new (reader->state, text);
            break;

        case TKN_HASH_U8_LP:
            datum = read_bytevector (reader);
            break;

        default:
            return R_UNSPECIFIED;
    }

    consume (reader);

    return datum;
}

static rsexp read_datum (RDatumReader* reader)
{
    if (lookahead (reader)->_id == TKN_TERMINATION)
        return R_EOF;

    if (lookahead (reader)->_id == TKN_HASH_SEMICOLON) {
        consume (reader);
        if (r_unspecified_p (read_datum (reader)))
            return R_UNSPECIFIED;
    }

    rsexp obj = read_simple_datum (reader);

    return r_unspecified_p (obj) ? read_compound_datum (reader) : obj;
}

rsexp r_read (RDatumReader* reader)
{
    rsexp datum;

    if (setjmp (reader->jmp))
        return R_UNSPECIFIED;

    datum = read_datum (reader);

    return r_unspecified_p (datum) ? R_EOF : datum;
}

RDatumReader* r_reader_new (RState* state)
{
    RDatumReader* reader = GC_NEW (RDatumReader);

    memset (reader, 0, sizeof (RDatumReader));

    reader->state      = state;
    reader->input_port = r_current_input_port (state);
    reader->last_error = R_UNDEFINED;
    reader->lexer      = lexer_new ();
    reader->lookahead  = NULL;

    QUEX_NAME (token_p_set) (reader->lexer, &reader->token);

    return reader;
}

RDatumReader* r_file_reader (RState* state, char const* filename)
{
    return r_port_reader (state, r_open_input_file (state, filename));
}

RDatumReader* r_string_reader (RState* state, char const* string)
{
    return r_port_reader (state, r_open_input_string (state, string));
}

RDatumReader* r_port_reader (RState* state, rsexp port)
{
    RDatumReader* reader = r_reader_new (state);
    reader->input_port = port;
    return reader;
}

rsexp r_reader_last_error (RDatumReader* reader)
{
    return reader->last_error;
}
