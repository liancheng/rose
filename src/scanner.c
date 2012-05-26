#include "opaque.h"
#include "scanner.h"

#include "rose/port.h"

struct RScanner {
    RLexer* lexer;
    RToken* lookahead_token;
};

static void scanner_finalize (rpointer scanner, rpointer client_data);

static RScanner* get_scanner (rsexp context);

RScanner* r_scanner_new ()
{
    RScanner* scanner = GC_NEW (RScanner);

    scanner->lexer = malloc (sizeof (RLexer));
    QUEX_NAME (construct_memory) (scanner->lexer, NULL, 0, NULL, NULL, FALSE);
    scanner->lookahead_token = NULL;

    GC_REGISTER_FINALIZER (scanner, scanner_finalize, NULL, NULL, NULL);

    return scanner;
}

static void scanner_finalize (rpointer obj, rpointer client_data)
{
    RScanner* scanner = obj;

    QUEX_NAME (destruct) (scanner->lexer);
    free (scanner->lexer);

    if (!scanner->lookahead_token)
        r_scanner_free_token (scanner->lookahead_token);

    free (scanner);
}

static rint reload_lexer (rsexp port, RLexer* lex)
{
    QUEX_NAME (buffer_fill_region_prepare) (lex);

    char* begin = (char*) QUEX_NAME (buffer_fill_region_begin) (lex);
    rint  size  = QUEX_NAME (buffer_fill_region_size) (lex);
    char* line  = r_port_gets (port, begin, size);
    rint  len   = line ? strlen (line) : 0;

    QUEX_NAME (buffer_fill_region_finish) (lex, len);

    return len;
}

void debug_token (RToken* token)
{
    printf ("id=%d name=%s text=[%s]\n",
            token->_id,
            QUEX_NAME_TOKEN (map_id_to_name) (token->_id),
            token->text);
}

static RToken* read_token (rsexp port, RScanner* scanner)
{
    RToken* t;

    QUEX_NAME (receive) (scanner->lexer, &t);

    while (TKN_EXPECT_MORE == t->_id) {
        if (0 == reload_lexer (port, scanner->lexer)) {
            quex_token_set (t, TKN_EOF);
            break;
        }

        QUEX_NAME (receive) (scanner->lexer, &t);
    }

    // debug_token (t);
    return r_scanner_copy_token (t);
}

static RScanner* get_scanner (rsexp context)
{
    return r_opaque_get (r_context_get (context, CTX_SCANNER));
}

void r_scanner_init (rsexp port, rsexp context)
{
    RScanner* scanner = get_scanner (context);
    reload_lexer (port, scanner->lexer);
}

RToken* r_scanner_copy_token (RToken* token)
{
    RToken* res = malloc (sizeof (RToken));
    QUEX_NAME_TOKEN (copy_construct) (res, token);
    return res;
}

// The caller is responsible for freeing the returned token.
RToken* r_scanner_next_token (rsexp port, rsexp context)
{
    RScanner* scanner = get_scanner (context);
    RToken* res = scanner->lookahead_token;

    if (res)
        scanner->lookahead_token = NULL;
    else
        res = read_token (port, scanner);

    return res;
}

// The caller must not free the returned token.
RToken* r_scanner_peek_token (rsexp port, rsexp context)
{
    RScanner* scanner = get_scanner (context);

    if (!scanner->lookahead_token)
        scanner->lookahead_token = read_token (port, scanner);

    return scanner->lookahead_token;
}

rtokenid r_scanner_peek_id (rsexp port, rsexp context)
{
    return r_scanner_peek_token (port, context)->_id;
}

void r_scanner_consume_token (rsexp port, rsexp context)
{
    r_scanner_free_token (r_scanner_next_token (port, context));
}

void r_scanner_free_token (RToken* token)
{
    QUEX_NAME_TOKEN (destruct) (token);
    free (token);
}
