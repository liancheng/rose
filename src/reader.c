#include "detail/context.h"
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
        r_keyword (id, reader->context)

#define r_throw longjmp (reader->jmp, 1)

typedef rsexp (*RDatumReaderRule) (RDatumReader*);

rsexp r_read_datum (RDatumReader* reader);

static rint reload_lexer (RLexer* lexer, rsexp port)
{
    QUEX_NAME (buffer_fill_region_prepare) (lexer);

    char* begin = (char*) QUEX_NAME (buffer_fill_region_begin) (lexer);
    rint  size  = QUEX_NAME (buffer_fill_region_size) (lexer);
    char* line  = r_port_gets (port, begin, size);
    rint  len   = line ? strlen (line) : 0;

    QUEX_NAME (buffer_fill_region_finish) (lexer, len);

    return len;
}

static RToken* copy_token (RToken* token)
{
    RToken* copy = malloc (sizeof (RToken));
    QUEX_NAME_TOKEN (copy_construct) (copy, token);
    return copy;
}

static void free_token (RToken* token)
{
    QUEX_NAME_TOKEN (destruct) (token);
    free (token);
}

static RToken* r_reader_next_token (RDatumReader* reader)
{
    RToken* token = NULL;

    QUEX_NAME (receive) (reader->lexer, &token);

    while (TKN_TERMINATION == token->_id) {
        if (0 == reload_lexer (reader->lexer, reader->input_port))
            break;

        QUEX_NAME (receive) (reader->lexer, &token);
    }

    return copy_token (token);
}

static void r_reader_consume (RDatumReader* reader)
{
    if (reader->lookahead) {
        free_token (reader->lookahead);
        reader->lookahead = NULL;
    }
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
    RLexer* lexer;

    lexer = GC_NEW (RLexer);
    QUEX_NAME (construct_memory) (lexer, NULL, 0, NULL, NULL, FALSE);
    GC_REGISTER_FINALIZER (lexer, r_lexer_finalize, NULL, NULL, NULL);

    return lexer;
}

static void r_reader_finalize (rpointer obj, rpointer client_data)
{
    RDatumReader* reader = obj;

    if (reader->lookahead)
        free_token (reader->lookahead);
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

static rsexp alternative (RDatumReader* reader, ...)
{
    va_list args;
    rsexp   obj;

    va_start (args, reader);

    for (obj = R_UNSPECIFIED; r_unspecified_p (obj); ) {
        RDatumReaderRule rule = va_arg (args, RDatumReaderRule);

        if (!rule)
            break;

        obj = rule (reader);
    }

    va_end (args);

    return obj;
}

void throw_error (RDatumReader* reader,
                  RToken*       token,
                  char const*   message,
                  ...)
{
    rsexp port   = r_port_get_name (reader->input_port);
    rsexp line   = r_int_to_sexp (token->_line_n);
    rsexp column = r_int_to_sexp (token->_column_n);

    reader->last_error = r_error_new (r_string_new (message),
                                      r_list (3, port, line, column));

    longjmp (reader->jmp, 1);
}

RDatumReader* r_reader_new (RContext* context)
{
    RDatumReader* reader = GC_NEW (RDatumReader);

    memset (reader, 0, sizeof (RDatumReader));

    reader->context    = context;
    reader->input_port = r_current_input_port (context);
    reader->last_error = R_UNDEFINED;
    reader->lexer      = r_lexer_new ();
    reader->lookahead  = NULL;

    GC_REGISTER_FINALIZER (reader, r_reader_finalize, NULL, NULL, NULL);

    return reader;
}

RDatumReader* r_file_reader (char const* filename, RContext* context)
{
    return r_port_reader (r_open_input_file (filename, context), context);
}

RDatumReader* r_string_reader (char const* string, RContext* context)
{
    return r_port_reader (r_open_input_string (string, context), context);
}

RDatumReader* r_port_reader (rsexp port, RContext* context)
{
    RDatumReader* reader = r_reader_new (context);
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

    MATCH_OR_RETURN_UNSPECIFIED (TKN_HASH_LP);
    CONSUME;

    for (list = R_NULL; LOOKAHEAD_ID != TKN_RP; ) {
        datum = r_read_datum (reader);

        if (r_unspecified_p (datum)) {
            r_throw;
        }

        list = r_cons (datum, list);
    }

    CONSUME;

    return r_list_to_vector (r_reverse (list));
}

rsexp r_read_full_list (RDatumReader* reader)
{
    rsexp list;
    rsexp datum;

    MATCH_OR_RETURN_UNSPECIFIED (TKN_LP);
    CONSUME;

    if (LOOKAHEAD_ID == TKN_RP) {
        CONSUME;
        return R_NULL;
    }

    datum = r_read_datum (reader);

    if (r_unspecified_p (datum)) {
        r_throw;
    }

    list = r_cons (datum, R_NULL);

    while (LOOKAHEAD_ID != TKN_DOT && LOOKAHEAD_ID != TKN_RP) {
        datum = r_read_datum (reader);

        if (r_unspecified_p (datum)) {
            r_throw;
        }

        list = r_cons (datum, list);
    }

    list = r_reverse (list);

    if (LOOKAHEAD_ID == TKN_DOT) {
        CONSUME;

        datum = r_read_datum (reader);

        if (r_unspecified_p (datum)) {
            r_throw;
        }

        list = r_append_x (list, datum);
    }

    if (LOOKAHEAD_ID != TKN_RP) {
        r_throw;
    }

    CONSUME;

    return list;
}

rsexp r_read_abbreviation (RDatumReader* reader)
{
    rsexp prefix;
    rsexp datum;

    switch (LOOKAHEAD_ID) {
        case TKN_QUOTE:    prefix = KEYWORD (R_QUOTE);            break;
        case TKN_BACKTICK: prefix = KEYWORD (R_QUASIQUOTE);       break;
        case TKN_COMMA:    prefix = KEYWORD (R_UNQUOTE);          break;
        case TKN_COMMA_AT: prefix = KEYWORD (R_UNQUOTE_SPLICING); break;

        default:
            return R_UNSPECIFIED;
    }

    CONSUME;

    datum = r_read_datum (reader);

    return r_unspecified_p (datum) ? R_UNSPECIFIED : r_list (2, prefix, datum);
}

rsexp r_read_list (RDatumReader* reader)
{
    return alternative (reader,
                        r_read_abbreviation,
                        r_read_full_list,
                        NULL);
}

rsexp r_read_compound_datum (RDatumReader* reader)
{
    return alternative (reader,
                        r_read_list,
                        r_read_vector,
                        NULL);
}

rsexp r_read_bytevector (RDatumReader* reader)
{
    rsexp bytes;
    rsexp datum;

    MATCH_OR_RETURN_UNSPECIFIED (TKN_HASH_U8_LP);
    CONSUME;

    bytes = R_NULL;

    while (LOOKAHEAD_ID != TKN_RP) {
        datum = r_read_datum (reader);

        if (!r_byte_p (datum)) {
            r_throw;
        }

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
            datum = r_symbol_new (text, reader->context);
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

    return alternative (reader,
                        r_read_simple_datum,
                        r_read_compound_datum,
                        NULL);
}

rsexp r_read (RDatumReader* reader)
{
    rsexp datum;

    if (setjmp (reader->jmp))
        return R_EOF;

    datum = r_read_datum (reader);

    return r_unspecified_p (datum) ? R_EOF : datum;
}
