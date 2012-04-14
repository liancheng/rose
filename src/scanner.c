#include "rose/context.h"
#include "rose/scanner.h"

#include <glib.h>

r_token* read_token(FILE* input, r_context* context)
{
    r_lexer* lex = (r_lexer*)(context->lexer);

    // Receive next token from the input stream.
    r_token_id id = QUEX_NAME(receive)(lex);

    // Reload the analyzer if there's no more token.
    if (TKN_TERMINATION == id) {
        QUEX_NAME(buffer_fill_region_prepare)(lex);

        // Read input from stream into analyzer buffer.
        char* begin = (char*)QUEX_NAME(buffer_fill_region_begin)(lex);
        int   size  = QUEX_NAME(buffer_fill_region_size)(lex);
        char* line  = fgets(begin, size, input);

        // EOF, no more input.
        if (!line) {
            QUEX_NAME(buffer_fill_region_finish)(lex, 0);
            return NULL;
        }

        QUEX_NAME(buffer_fill_region_finish)(lex, strlen(line));

        // Discard the last TKN_TERMINATION token,
        // try to receive another token.
        QUEX_NAME(receive)(lex);
    }

    // Copy the token and return it.
    r_token* t = (r_token*)malloc(sizeof(r_token));
    QUEX_NAME_TOKEN(copy_construct)(t, QUEX_NAME(token_p)(lex));

    return t;
}

/* Caller must free the token. */
r_token* scanner_next_token(FILE* input, r_context* context)
{
    GQueue* q = (GQueue*)(context->token_queue);
    return (g_queue_is_empty(q)) ? read_token(input, context)
                                 : (r_token*)g_queue_pop_head(q);
}

/* Caller must *NOT* free the token. */
r_token* scanner_peek_token(FILE* input, r_context* context)
{
    r_token* t = NULL;
    GQueue*  q = (GQueue*)(context->token_queue);

    if (g_queue_is_empty(q)) {
        if ((t = read_token(input, context))) {
            g_queue_push_tail(q, t);
        }
    }
    else {
        t = (r_token*)g_queue_peek_head(q);
    }

    return t;
}

r_token_id scanner_peek_token_id(FILE* input, r_context* context)
{
    return scanner_peek_token(input, context)->_id;
}
