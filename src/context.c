#include "rose/scanner.h"
#include "rose/context.h"

#include <glib.h>

r_context* context_new()
{
    r_context* context = (r_context*)malloc(sizeof(r_context));

    // Initialize the Quex lexer.
    r_lexer* lexer = (r_lexer*)malloc(sizeof(r_lexer));
    QUEX_NAME(construct_memory)(lexer, NULL, 0, NULL, NULL, false);
    context->lexer = (void*)lexer;

    // Initialize the token queue (the parser uses it for lookahead).
    context->token_queue = (void*)g_queue_new();

    return context;
}

void context_free(r_context* context)
{
    // Destroy the Quex lexer.
    QUEX_NAME(destruct)((r_lexer*)(context->lexer));
    free(context->lexer);

    // Destroy all tokens left in the token queue.
    r_token* t = NULL;
    while ((t = g_queue_pop_head((GQueue*)(context->token_queue))))
        QUEX_NAME_TOKEN(destruct)(t);

    // Destroy the token queue.
    g_queue_free(context->token_queue);

    free(context);
}
