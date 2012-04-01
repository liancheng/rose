#include "sexp.h"
#include "lexer/r5rs_lexer.h"

#include <stdio.h>
#include <stdlib.h>

void handle_token(QUEX_TYPE_TOKEN token)
{
    char buffer[1024];
    printf("%s \n", QUEX_NAME_TOKEN(get_string)(&token, buffer, 1024));
}

int main(int argc, char* argv[])
{
    QUEX_TYPE_TOKEN token;
    QUEX_NAME_TOKEN(construct)(&token);

    r5rs_lexer qlex;
    QUEX_NAME(construct_memory)(&qlex, NULL, 0, NULL, NULL, false);

    (void)QUEX_NAME(token_p_switch)(&qlex, &token);

    size_t received_n = (size_t)-1;
    while (0 != received_n) {
        QUEX_NAME(buffer_fill_region_prepare)(&qlex);

        char* qlex_buffer = (char*)QUEX_NAME(buffer_fill_region_begin)(&qlex);
        size_t qlex_buffer_size = QUEX_NAME(buffer_fill_region_size(&qlex));

        if (NULL == fgets(qlex_buffer, qlex_buffer_size, stdin) ) {
            break;
        }

        received_n = strlen(qlex_buffer);
        QUEX_NAME(buffer_fill_region_finish)(&qlex, received_n);

        do {
            QUEX_NAME(receive)(&qlex);
            handle_token(token);
        }
        while (TKN_TERMINATION != token._id && TKN_FAIL != token._id);
    }

    return 0;
}
