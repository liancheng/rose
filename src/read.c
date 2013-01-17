#include "detail/env.h"
#include "detail/read.h"
#include "detail/state.h"
#include "rose/bytevector.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/opaque.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/primitive.h"
#include "rose/read.h"
#include "rose/string.h"
#include "rose/vector.h"

#include <alloca.h>
#include <assert.h>

static rsexp read_datum (Reader* reader);

static rint feed_lexer (Reader* reader)
{
    RState* r;
    rcstring begin;
    rcstring line;
    rint size;
    rint len;

    QUEX_NAME (buffer_fill_region_prepare) (&reader->lexer);

    begin = r_cast (rcstring,
                    QUEX_NAME (buffer_fill_region_begin)
                               (&reader->lexer));

    r = reader->r;
    size = QUEX_NAME (buffer_fill_region_size) (&reader->lexer);
    line = r_port_gets (r, reader->input_port, begin, size);
    len = line ? strlen (line) : 0;

    QUEX_NAME (buffer_fill_region_finish) (&reader->lexer, len);

    return len;
}

static RToken* next_token (Reader* reader)
{
    rtokenid id;
    RToken*  token;

    do
        id = QUEX_NAME (receive) (&reader->lexer);
    while (TKN_TERMINATION == id && feed_lexer (reader));

    token = QUEX_NAME (token_p) (&reader->lexer);
    reader->current_line = token->_line_n;
    reader->current_column = token->_column_n;

    return token;
}

static void consume (Reader* reader)
{
    if (reader->lookahead)
        reader->lookahead = NULL;
}

static RToken* lookahead (Reader* reader)
{
    if (!reader->lookahead)
        reader->lookahead = next_token (reader);

    return reader->lookahead;
}

static rbool match (Reader* reader, rtokenid id)
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

static void syntax_error (Reader* reader, char const* message)
{
    rsexp error;

    r_gc_scope_open (reader->r);

    error = r_error_format (reader->r,
                            "~a:~a:~a: ~a~%",
                            r_port_get_name (reader->input_port),
                            r_int_to_sexp (reader->current_line),
                            r_int_to_sexp (reader->current_column),
                            r_string_new (reader->r, message));

    r_gc_scope_close_and_protect (reader->r, error);
}

static rsexp read_vector (Reader* reader)
{
    rsexp datum;
    rsexp list;
    rsexp res;

    res = R_FAILURE;

    if (!match (reader, TKN_HASH_LP))
        goto exit;

    r_gc_scope_open (reader->r);

    for (list = R_NULL; lookahead (reader)->_id != TKN_RP; ) {
        datum = read_datum (reader);

        if (r_eof_object_p (datum)) {
            syntax_error (reader, "the vector is not closed");
            res = R_FAILURE;
            goto clean;
        }

        if (r_failure_p (datum)) {
            syntax_error (reader, "expecting a vector element");
            res = R_FAILURE;
            goto clean;
        }

        ensure_or_goto (list = r_cons (reader->r, datum, list), clean);
    }

    consume (reader);

    res = r_list_to_vector (reader->r, r_reverse_x (reader->r, list));

clean:
    r_gc_scope_close_and_protect (reader->r, res);

exit:
    return res;
}

static rsexp read_full_list (Reader* reader)
{
    rsexp res;
    rsexp datum;

    res = R_FAILURE;

    if (!match (reader, TKN_LP))
        goto exit;

    if (match (reader, TKN_RP)) {
        res = R_NULL;
        goto exit;
    }

    datum = read_datum (reader);
    if (r_failure_p (datum)) {
        syntax_error (reader, "bad syntax");
        goto exit;
    }

    r_gc_scope_open (reader->r);

    ensure_or_goto (res = r_cons (reader->r, datum, R_NULL), clean);

    while (TRUE) {
        rtokenid id = lookahead (reader)->_id;

        if (id == TKN_DOT || id == TKN_RP)
            break;

        datum = read_datum (reader);

        if (r_failure_p (datum)) {
            syntax_error (reader, "bad syntax");
            res = R_FAILURE;
            goto clean;
        }

        ensure_or_goto (res = r_cons (reader->r, datum, res), clean);
    }

    res = r_reverse_x (reader->r, res);

    if (match (reader, TKN_DOT)) {
        datum = read_datum (reader);

        if (r_failure_p (datum)) {
            syntax_error (reader, "datum expected");
            res = R_FAILURE;
            goto clean;
        }

        res = r_append_x (reader->r, res, datum);
    }

    if (!match (reader, TKN_RP)) {
        syntax_error (reader, "missing close parenthesis");
        res = R_FAILURE;
    }

clean:
    r_gc_scope_close_and_protect (reader->r, res);

exit:
    return res;
}

static rsexp read_abbreviation (Reader* reader)
{
    RState* r;
    rsexp prefix;
    rsexp datum;
    rsexp res;

    r = reader->r;
    res = R_FAILURE;

    switch (lookahead (reader)->_id) {
        case TKN_QUOTE:
            prefix = r->kw.quote;
            break;

        case TKN_BACKTICK:
            prefix = r->kw.quasiquote;
            break;

        case TKN_COMMA:
            prefix = r->kw.unquote;
            break;

        case TKN_COMMA_AT:
            prefix = r->kw.unquote_splicing;
            break;

        default:
            goto exit;
    }

    consume (reader);

    datum = read_datum (reader);
    if (r_failure_p (datum)) {
        syntax_error (reader, "bad syntax");
        goto exit;
    }

    res = r_list (r, 2, prefix, datum);

exit:
    return res;
}

static rsexp read_list (Reader* reader)
{
    return TKN_LP == lookahead (reader)->_id
           ? read_full_list (reader)
           : read_abbreviation (reader);
}

static rsexp read_compound_datum (Reader* reader)
{
    return TKN_HASH_LP == lookahead (reader)->_id
           ? read_vector (reader)
           : read_list (reader);
}

static rsexp read_bytevector (Reader* reader)
{
    rsexp bytes;
    rsexp datum;
    rsexp res = R_FAILURE;

    if (!match (reader, TKN_HASH_U8_LP))
        goto exit;

    bytes = R_NULL;

    while (lookahead (reader)->_id != TKN_RP) {
        datum = read_datum (reader);

        if (!r_byte_p (datum)) {
            syntax_error (reader, "value out of range");
            goto exit;
        }

        ensure_or_goto (bytes = r_cons (reader->r, datum, bytes), exit);
    }

    if (!match (reader, TKN_RP))
        goto exit;

    res = r_list_to_bytevector (reader->r,
                                r_reverse_x (reader->r, bytes));

exit:
    return res;
}

static rsexp read_simple_datum (Reader* reader)
{
    RToken* token;
    rsexp datum;
    rcstring text;
    rsize size;

    token = lookahead (reader);
    size = QUEX_NAME (strlen) (token->text) + 5;
    text = alloca (size);

    QUEX_NAME_TOKEN (pretty_char_text) (token, text, size);

    switch (token->_id) {
        case TKN_TRUE:
            datum = R_TRUE;
            break;

        case TKN_FALSE:
            datum = R_FALSE;
            break;

        case TKN_NUMBER:
            datum = r_string_to_number (reader->r, text);
            break;

        case TKN_CHARACTER:
            datum = lexeme_to_char (text);
            break;

        case TKN_STRING:
            datum = r_string_new (reader->r, text);
            break;

        case TKN_IDENTIFIER:
            datum = r_intern (reader->r, text);
            break;

        case TKN_HASH_U8_LP:
            datum = read_bytevector (reader);
            break;

        default:
            datum = R_FAILURE;
            goto exit;
    }

    consume (reader);

exit:
    return datum;
}

static rsexp read_datum (Reader* reader)
{
    rsexp datum;

    if (match (reader, TKN_HASH_SEMICOLON))
        if (r_failure_p (read_datum (reader)))
            return R_FAILURE;

    datum = read_simple_datum (reader);

    return r_failure_p (datum) ? read_compound_datum (reader) : datum;
}

static void reader_finalize (RState* r, rpointer obj)
{
    Reader* reader = r_cast (Reader*, obj);

    assert (r == reader->r);

    QUEX_NAME (destruct) (&reader->lexer);
    QUEX_NAME_TOKEN (destruct) (&reader->token);

    r_free (r, reader);
}

static void reader_mark (RState* r, rpointer obj)
{
    r_gc_mark (r, r_cast (Reader*, obj)->input_port);
}

rsexp r_reader_new (RState* r, rsexp port)
{
    Reader* reader;
    rsexp res = R_FAILURE;

    reader = r_new0 (r, Reader);

    if (!reader)
        goto exit;

    reader->r              = r;
    reader->input_port     = port;
    reader->lookahead      = NULL;
    reader->current_line   = 0;
    reader->current_column = 0;

    QUEX_NAME (construct_memory) (&reader->lexer, NULL, 0, NULL, NULL, FALSE);
    QUEX_NAME_TOKEN (construct) (&reader->token);
    QUEX_NAME (token_p_set) (&reader->lexer, &reader->token);

    res = r_opaque_new (r, reader, reader_mark, reader_finalize);

    if (r_failure_p (res))
        r_free (r, reader);

exit:
    return res;
}

rsexp r_read (rsexp reader)
{
    Reader* r = r_cast (Reader*, r_opaque_get (reader));
    return lookahead (r)->_id == TKN_TERMINATION ? R_EOF : read_datum (r);
}

rsexp r_read_from_string (RState* r, rconstcstring input)
{
    rsexp str = r_string_new (r, input);
    rsexp port = r_open_input_string (r, str);
    rsexp reader = r_reader_new (r, port);

    return r_read (reader);
}

rsexp np_read (RState* r, rsexp args)
{
    rsexp port;
    rsexp reader;

    r_match_args (r, args, 0, 1, FALSE, &port);

    if (r_undefined_p (port))
        port = r_current_input_port (r);

    ensure (reader = r_reader_new (r, port));

    return r_read (reader);
}

RPrimitiveDesc read_primitives [] = {
    { "read", np_read, 0, 1, FALSE },
    { NULL }
};
