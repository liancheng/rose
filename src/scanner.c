#include "rose/context.h"
#include "rose/scanner.h"

#include <glib.h>

static void reload_lexer(r_lexer* lex, FILE* input)
{
    QUEX_NAME(buffer_fill_region_prepare)(lex);

    char* begin = (char*)QUEX_NAME(buffer_fill_region_begin)(lex);
    int   size  = QUEX_NAME(buffer_fill_region_size)(lex);
    char* line  = fgets(begin, size, input);
    int   len   = line ? strlen(line) : 0;

    QUEX_NAME(buffer_fill_region_finish)(lex, len);
}

static r_token* read_token(FILE* input, r_context* context)
{
    r_token* t = NULL;
    QUEX_NAME(receive)(context->lexer, &t);

    if (TKN_TERMINATION == t->_id) {
        reload_lexer(context->lexer, input);
        QUEX_NAME(receive)(context->lexer, &t);
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
    return g_queue_is_empty(context->token_queue)
           ? read_token(input, context)
           : (r_token*)g_queue_pop_head(context->token_queue);
}

// The caller must not free the returned token.
r_token* scanner_peek_token(FILE* input, r_context* context)
{
    GQueue* q = (GQueue*)(context->token_queue);

    if (g_queue_is_empty(q))
        g_queue_push_tail(q, read_token(input, context));

    return (r_token*)g_queue_peek_head(q);
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
