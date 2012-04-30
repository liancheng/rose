#include "rose/context.h"
#include "rose/scanner.h"

r_scanner* scanner_new()
{
    r_scanner* res = malloc(sizeof(r_scanner));

    res->lexer = malloc(sizeof(r_lexer));
    QUEX_NAME(construct_memory)(res->lexer, NULL, 0, NULL, NULL, false);

    return res;
}

void scanner_free(r_scanner* scanner)
{
    QUEX_NAME(destruct)(scanner->lexer);
    free(scanner->lexer);

    if (!scanner->lookahead_token)
        scanner_free_token(scanner->lookahead_token);

    free(scanner);
}

static void reload_lexer(FILE* input, r_lexer* lex)
{
    QUEX_NAME(buffer_fill_region_prepare)(lex);

    char* begin = (char*)QUEX_NAME(buffer_fill_region_begin)(lex);
    int   size  = QUEX_NAME(buffer_fill_region_size)(lex);
    char* line  = fgets(begin, size, input);
    int   len   = line ? strlen(line) : 0;

    QUEX_NAME(buffer_fill_region_finish)(lex, len);
}

static void debug_token(r_token* t, char const* prefix)
{
    fprintf(stderr,
            "%s(%d:%d) id=%s text=[%s]\n",
            prefix,
            t->_line_n,
            t->_column_n,
            QUEX_NAME_TOKEN(map_id_to_name)(t->_id),
            t->text);
}

static r_token* read_token(FILE* input, r_scanner* scanner)
{
    r_token* t = NULL;

    QUEX_NAME(receive)(scanner->lexer, &t);
    debug_token(t, "");

    if (TKN_TERMINATION == t->_id) {
        reload_lexer(input, scanner->lexer);
        QUEX_NAME(receive)(scanner->lexer, &t);
        debug_token(t, " ");
    }

    return scanner_copy_token(t);
}

void scanner_init(FILE* input, r_context* context)
{
    reload_lexer(input, context->scanner->lexer);
}

r_token* scanner_copy_token(r_token* token)
{
    r_token* res = malloc(sizeof(r_token));
    QUEX_NAME_TOKEN(copy_construct)(res, token);
    return res;
}

// The caller is responsible for freeing the returned token.
r_token* scanner_next_token(FILE* input, r_context* context)
{
    r_scanner* scanner = context->scanner;
    r_token* res = scanner->lookahead_token;

    if (res)
        scanner->lookahead_token = NULL;
    else
        res = read_token(input, scanner);

    return res;
}

// The caller must not free the returned token.
r_token* scanner_peek_token(FILE* input, r_context* context)
{
    r_scanner* scanner = context->scanner;

    if (!scanner->lookahead_token)
        scanner->lookahead_token = read_token(input, scanner);

    return scanner->lookahead_token;
}

r_token_id scanner_peek_token_id(FILE* input, r_context* context)
{
    return scanner_peek_token(input, context)->_id;
}

void scanner_consume_token(FILE* input, r_context* context)
{
    scanner_free_token(scanner_next_token(input, context));
}

void scanner_free_token(r_token* token)
{
    QUEX_NAME_TOKEN(destruct)(token);
    free(token);
}
