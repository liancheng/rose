#include "lexer.h"

static lexer* get_lexer()
{
    static lexer qlex;
    return &qlex;
}

lexer* lexer_init()
{
    // Initialize the Quex lexer, which is a global object.
    QUEX_NAME(construct_memory)(get_lexer(), NULL, 0, NULL, NULL, false);
    return get_lexer();
}

void lexer_finish()
{
    QUEX_NAME(destruct)(get_lexer());
}

token* read_token(FILE* file)
{
    lexer* qlex = get_lexer();

    token_id id = QUEX_NAME(receive)(qlex);

    // Try to reload the analyzer with more input.
    if (TKN_TERMINATION == id) {
        QUEX_NAME(buffer_fill_region_prepare)(qlex);

        // Read input from stream into analyzer buffer.
        char* begin = (char*)QUEX_NAME(buffer_fill_region_begin)(qlex);
        int   size  = QUEX_NAME(buffer_fill_region_size)(qlex);
        char* line  = fgets(begin, size, file);

        // EOF, no more input.
        if (!line) {
            QUEX_NAME(buffer_fill_region_finish)(qlex, 0);
            return NULL;
        }

        QUEX_NAME(buffer_fill_region_finish)(qlex, strlen(line));

        // Discard the last TKN_TERMINATION token,
        // get next available token. 
        QUEX_NAME(receive)(qlex);
    }

    return QUEX_NAME(token_p)(qlex);
}
