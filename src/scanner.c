#include "rose/context.h"
#include "rose/scanner.h"

#include <glib.h>

r_scanner* scanner_new()
{
    r_scanner* res = malloc(sizeof(r_scanner));

    res->lexer = malloc(sizeof(r_lexer));
    QUEX_NAME(construct_memory)(res->lexer, NULL, 0, NULL, NULL, false);

    res->prev_token = NULL;
    res->prev_pos = NULL;
    res->token_queue = g_queue_new();

    return res;
}

void scanner_free(r_scanner* scanner)
{
    QUEX_NAME(destruct)(scanner->lexer);
    free(scanner->lexer);

    while (!g_queue_is_empty(scanner->token_queue))
        QUEX_NAME_TOKEN(destruct)(g_queue_pop_head(scanner->token_queue));

    g_queue_free(scanner->token_queue);

    free(scanner);
}

static void reload_lexer(r_lexer* lex, FILE* input)
{
    QUEX_NAME(buffer_fill_region_prepare)(lex);

    char* begin = (char*)QUEX_NAME(buffer_fill_region_begin)(lex);
    int   size  = QUEX_NAME(buffer_fill_region_size)(lex);
    char* line  = fgets(begin, size, input);
    int   len   = line ? strlen(line) : 0;

    QUEX_NAME(buffer_fill_region_finish)(lex, len);
}

static r_token* read_token(FILE* input, r_lexer* lexer)
{
    r_token* t = NULL;

    QUEX_NAME(receive)(lexer, &t);

    if (TKN_TERMINATION == t->_id) {
        reload_lexer(lexer, input);
        QUEX_NAME(receive)(lexer, &t);
    }

    return scanner_copy_token(t);
}

r_token* scanner_copy_token(r_token* token)
{
    r_token* res = (r_token*)malloc(sizeof(r_token));
    QUEX_NAME_TOKEN(copy_construct)(res, token);
    return res;
}

// The caller is responsible for freeing the returned token.
r_token* scanner_next_token(FILE* input, r_context* context)
{
    return g_queue_is_empty(context->scanner->token_queue)
           ? read_token(input, context->scanner->lexer)
           : g_queue_pop_head(context->scanner->token_queue);
}

// The caller must not free the returned token.
r_token* scanner_peek_token(FILE* input, r_context* context)
{
    if (g_queue_is_empty(context->scanner->token_queue))
        g_queue_push_tail(context->scanner->token_queue,
                          read_token(input,
                                     context->scanner->lexer));

    return g_queue_peek_head(context->scanner->token_queue);
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
