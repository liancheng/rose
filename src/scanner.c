#include "rose/context.h"
#include "rose/scanner.h"

struct RScanner {
    RLexer* lexer;
    RToken* lookahead_token;
};

RScanner* r_scanner_new()
{
    RScanner* res = malloc(sizeof(RScanner));

    res->lexer = malloc(sizeof(RLexer));
    QUEX_NAME(construct_memory)(res->lexer, NULL, 0, NULL, NULL, false);

    return res;
}

void r_scanner_free(RScanner* scanner)
{
    QUEX_NAME(destruct)(scanner->lexer);
    free(scanner->lexer);

    if (!scanner->lookahead_token)
        r_scanner_free_token(scanner->lookahead_token);

    free(scanner);
}

static void reload_lexer(FILE* input, RLexer* lex)
{
    QUEX_NAME(buffer_fill_region_prepare)(lex);

    char* begin = (char*)QUEX_NAME(buffer_fill_region_begin)(lex);
    int   size  = QUEX_NAME(buffer_fill_region_size)(lex);
    char* line  = fgets(begin, size, input);
    int   len   = line ? strlen(line) : 0;

    QUEX_NAME(buffer_fill_region_finish)(lex, len);
}

static void debug_token(RToken* t, char const* prefix)
{
    fprintf(stderr,
            "%s(%d:%d) id=%s text=[%s]\n",
            prefix,
            t->_line_n,
            t->_column_n,
            QUEX_NAME_TOKEN(map_id_to_name)(t->_id),
            t->text);
}

static RToken* read_token(FILE* input, RScanner* scanner)
{
    RToken* t = NULL;

    QUEX_NAME(receive)(scanner->lexer, &t);
    debug_token(t, "");

    if (TKN_TERMINATION == t->_id) {
        reload_lexer(input, scanner->lexer);
        QUEX_NAME(receive)(scanner->lexer, &t);
        debug_token(t, " ");
    }

    return r_scanner_copy_token(t);
}

void r_scanner_init(FILE* input, RContext* context)
{
    RScanner* scanner = r_context_get_scanner(context);
    reload_lexer(input, scanner->lexer);
}

RToken* r_scanner_copy_token(RToken* token)
{
    RToken* res = malloc(sizeof(RToken));
    QUEX_NAME_TOKEN(copy_construct)(res, token);
    return res;
}

// The caller is responsible for freeing the returned token.
RToken* r_scanner_next_token(FILE* input, RContext* context)
{
    RScanner* scanner = r_context_get_scanner(context);
    RToken* res = scanner->lookahead_token;

    if (res)
        scanner->lookahead_token = NULL;
    else
        res = read_token(input, scanner);

    return res;
}

// The caller must not free the returned token.
RToken* r_scanner_peek_token(FILE* input, RContext* context)
{
    RScanner* scanner = r_context_get_scanner(context);

    if (!scanner->lookahead_token)
        scanner->lookahead_token = read_token(input, scanner);

    return scanner->lookahead_token;
}

rtokenid r_scanner_peek_token_id(FILE* input, RContext* context)
{
    return r_scanner_peek_token(input, context)->_id;
}

void r_scanner_consume_token(FILE* input, RContext* context)
{
    r_scanner_free_token(r_scanner_next_token(input, context));
}

void r_scanner_free_token(RToken* token)
{
    QUEX_NAME_TOKEN(destruct)(token);
    free(token);
}
