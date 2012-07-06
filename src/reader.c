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

#define CONSUME\
        r_reader_consume (reader)

#define LOOKAHEAD\
        r_reader_lookahead (reader)

#define LOOKAHEAD_ID\
        (r_reader_lookahead (reader)->_id)

#define MATCH_OR_RETURN_UNSPECIFIED(id)\
        do {\
            if (LOOKAHEAD_ID != id)\
                return R_UNSPECIFIED;\
        }\
        while (0)

#define KEYWORD(id)\
        r_keyword (reader->state, id)

#define SOURCE_LOCATION(line, column)\
        do {\
            (line)   = LOOKAHEAD->_line_n;\
            (column) = LOOKAHEAD->_column_n;\
        }\
        while (0)

rsexp r_read_datum (RDatumReader* reader);

static rint r_reader_reload_lexer (RDatumReader* reader)
{
    QUEX_NAME (buffer_fill_region_prepare) (reader->lexer);

    char* begin = (char*) QUEX_NAME (buffer_fill_region_begin) (reader->lexer);
    rint  size  = QUEX_NAME (buffer_fill_region_size) (reader->lexer);
    char* line  = r_port_gets (reader->input_port, begin, size);
    rint  len   = line ? strlen (line) : 0;

    QUEX_NAME (buffer_fill_region_finish) (reader->lexer, len);

    return len;
}

static RToken* r_reader_next_token (RDatumReader* reader)
{
    rtokenid id;

    do
        id = QUEX_NAME (receive) (reader->lexer);
    while (TKN_TERMINATION == id && r_reader_reload_lexer (reader));

    return QUEX_NAME (token_p) (reader->lexer);
}

static void r_reader_consume (RDatumReader* reader)
{
    if (reader->lookahead)
        reader->lookahead = NULL;
}

static RToken* r_reader_lookahead (RDatumReader* reader)
{
    if (!reader->lookahead)
        reader->lookahead = r_reader_next_token (reader);

    return reader->lookahead;
}

static void r_lexer_finalize (rpointer obj, rpointer client_data)
{
    QUEX_NAME (destruct) ((RLexer*) obj);
}

static RLexer* r_lexer_new ()
{
    RLexer* lexer = GC_NEW_ATOMIC (RLexer);

    QUEX_NAME (construct_memory) (lexer, NULL, 0, NULL, NULL, FALSE);
    GC_REGISTER_FINALIZER (lexer, r_lexer_finalize, NULL, NULL, NULL);

    return lexer;
}

static rsexp lexeme_to_char (char const* text)
{
    rint len = strlen (text);
    char res = *text;

    if (len > 1) {
        if      (0 == strcmp (text, "space"))     res = ' ';
        else if (0 == strcmp (text, "tab"))       res = '\t';
        else if (0 == strcmp (text, "newline"))   res = '\n';
        else if (0 == strcmp (text, "return"))    res = '\r';
        else if (0 == strcmp (text, "null"))      res = '\0';
        else if (0 == strcmp (text, "alarm"))     res = '\a';
        else if (0 == strcmp (text, "backspace")) res = '\b';
        else if (0 == strcmp (text, "escape"))    res = '\x1b';
        else if (0 == strcmp (text, "delete"))    res = '\x7f';
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

RDatumReader* r_reader_new (RState* state)
{
    RDatumReader* reader = GC_NEW (RDatumReader);

    memset (reader, 0, sizeof (RDatumReader));

    reader->state      = state;
    reader->input_port = r_current_input_port (state);
    reader->last_error = R_UNDEFINED;
    reader->lexer      = r_lexer_new ();
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

void r_reader_clear_error (RDatumReader* reader)
{
    reader->last_error = R_UNDEFINED;
}

rsexp r_read_vector (RDatumReader* reader)
{
    rsexp datum;
    rsexp list;
    rsize line;
    rsize column;

    MATCH_OR_RETURN_UNSPECIFIED (TKN_HASH_LP);

    CONSUME;

    SOURCE_LOCATION (line, column);

    for (list = R_NULL; LOOKAHEAD_ID != TKN_RP; ) {
        datum = r_read_datum (reader);

        if (r_unspecified_p (datum))
            raise_reader_error (reader, line, column, "bad syntax");

        list = r_cons (datum, list);
    }

    CONSUME;

    return r_list_to_vector (r_reverse (list));
}

rsexp r_read_full_list (RDatumReader* reader)
{
    rsexp list;
    rsexp datum;
    rsize line;
    rsize column;

    MATCH_OR_RETURN_UNSPECIFIED (TKN_LP);
    CONSUME;

    if (LOOKAHEAD_ID == TKN_RP) {
        CONSUME;
        return R_NULL;
    }

    SOURCE_LOCATION (line, column);

    datum = r_read_datum (reader);

    if (r_unspecified_p (datum))
        raise_reader_error (reader, line, column, "bad syntax");

    list = r_cons (datum, R_NULL);

    while (LOOKAHEAD_ID != TKN_DOT && LOOKAHEAD_ID != TKN_RP) {
        SOURCE_LOCATION (line, column);

        datum = r_read_datum (reader);

        if (r_unspecified_p (datum))
            raise_reader_error (reader, line, column, "bad syntax");

        list = r_cons (datum, list);
    }

    list = r_reverse (list);

    if (LOOKAHEAD_ID == TKN_DOT) {
        CONSUME;
        SOURCE_LOCATION (line, column);

        datum = r_read_datum (reader);

        if (r_unspecified_p (datum))
            raise_reader_error (reader, line, column, "missing close parenthesis");

        list = r_append_x (list, datum);
    }

    SOURCE_LOCATION (line, column);

    if (LOOKAHEAD_ID != TKN_RP)
        raise_reader_error (reader, line, column, "bad syntax");

    CONSUME;

    return list;
}

rsexp r_read_abbreviation (RDatumReader* reader)
{
    rsexp prefix;
    rsexp datum;
    rsize line;
    rsize column;

    switch (LOOKAHEAD_ID) {
        case TKN_QUOTE:    prefix = KEYWORD (R_QUOTE);            break;
        case TKN_BACKTICK: prefix = KEYWORD (R_QUASIQUOTE);       break;
        case TKN_COMMA:    prefix = KEYWORD (R_UNQUOTE);          break;
        case TKN_COMMA_AT: prefix = KEYWORD (R_UNQUOTE_SPLICING); break;

        default:
            return R_UNSPECIFIED;
    }

    CONSUME;
    SOURCE_LOCATION (line, column);

    datum = r_read_datum (reader);

    if (r_unspecified_p (datum))
        raise_reader_error (reader, line, column, "bad syntax");

    return r_list (2, prefix, datum);
}

rsexp r_read_list (RDatumReader* reader)
{
    rsexp obj = r_read_abbreviation (reader);

    return r_unspecified_p (obj) ? r_read_full_list (reader) : obj;
}

rsexp r_read_compound_datum (RDatumReader* reader)
{
    rsexp obj = r_read_list (reader);

    return r_unspecified_p (obj) ? r_read_vector (reader) : obj;
}

rsexp r_read_bytevector (RDatumReader* reader)
{
    rsexp bytes;
    rsexp datum;
    rsize line;
    rsize column;

    MATCH_OR_RETURN_UNSPECIFIED (TKN_HASH_U8_LP);
    CONSUME;

    bytes = R_NULL;

    while (LOOKAHEAD_ID != TKN_RP) {
        SOURCE_LOCATION (line, column);

        datum = r_read_datum (reader);

        if (!r_byte_p (datum))
            raise_reader_error (reader, line, column, "value out of range");

        bytes = r_cons (datum, bytes);
    }

    MATCH_OR_RETURN_UNSPECIFIED (TKN_RP);
    CONSUME;

    return r_list_to_bytevector (r_reverse (bytes));
}

rsexp r_read_simple_datum (RDatumReader* reader)
{
    RToken* token;
    rsexp   datum;
    char*   text;
    rsize   size;

    token = LOOKAHEAD;
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
            datum = r_read_bytevector (reader);
            break;

        default:
            return R_UNSPECIFIED;
    }

    CONSUME;

    return datum;
}

rsexp r_read_datum (RDatumReader* reader)
{
    if (LOOKAHEAD_ID == TKN_TERMINATION)
        return R_EOF;

    if (LOOKAHEAD_ID == TKN_HASH_SEMICOLON) {
        CONSUME;
        if (r_unspecified_p (r_read_datum (reader)))
            return R_UNSPECIFIED;
    }

    rsexp obj = r_read_simple_datum (reader);

    return r_unspecified_p (obj) ? r_read_compound_datum (reader) : obj;
}

rsexp r_read (RDatumReader* reader)
{
    rsexp datum;

    if (setjmp (reader->jmp))
        return R_UNSPECIFIED;

    datum = r_read_datum (reader);

    return r_unspecified_p (datum) ? R_EOF : datum;
}
