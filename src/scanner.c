#include "opaque.h"
#include "scanner.h"

#include "rose/port.h"

struct RScanner {
    RLexer* lexer;
    RToken* lookahead_token;
    int     line;
    int     column;
};

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

RScanner* r_scanner_new ()
{
    RScanner* scanner = GC_NEW (RScanner);
    memset (scanner, 0, sizeof (RScanner));

    scanner->lexer = malloc (sizeof (RLexer));
    QUEX_NAME (construct_memory) (scanner->lexer, NULL, 0, NULL, NULL, FALSE);
    scanner->lookahead_token = NULL;

    GC_REGISTER_FINALIZER (scanner, scanner_finalize, NULL, NULL, NULL);

    return scanner;
}

void debug_token (RToken* token)
{
    printf ("%d:%d id=%d name=%s text=[%s]\n",
            token->_line_n,
            token->_column_n,
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

    scanner->line   = t->_line_n;
    scanner->column = t->_column_n;

    debug_token (t);
    return r_scanner_copy_token (t);
}

void r_scanner_init (RScanner* scanner, rsexp port)
{
    reload_lexer (port, scanner->lexer);
}

RToken* r_scanner_copy_token (RToken* token)
{
    RToken* res = malloc (sizeof (RToken));
    QUEX_NAME_TOKEN (copy_construct) (res, token);
    return res;
}

// The caller is responsible for freeing the returned token.
RToken* r_scanner_next_token (RScanner* scanner, rsexp port)
{
    RToken* res = scanner->lookahead_token;

    if (res)
        scanner->lookahead_token = NULL;
    else
        res = read_token (port, scanner);

    return res;
}

// The caller must not free the returned token.
RToken* r_scanner_peek_token (RScanner* scanner, rsexp port)
{
    if (!scanner->lookahead_token)
        scanner->lookahead_token = read_token (port, scanner);

    return scanner->lookahead_token;
}

rtokenid r_scanner_peek_id (RScanner* scanner, rsexp port)
{
    return r_scanner_peek_token (scanner, port)->_id;
}

void r_scanner_consume_token (RScanner* scanner, rsexp port)
{
    r_scanner_free_token (r_scanner_next_token (scanner, port));
}

void r_scanner_free_token (RToken* token)
{
    QUEX_NAME_TOKEN (destruct) (token);
    free (token);
}

int r_scanner_line (RScanner* scanner)
{
    return scanner->line;
}

int r_scanner_column (RScanner* scanner)
{
    return scanner->column;
}
