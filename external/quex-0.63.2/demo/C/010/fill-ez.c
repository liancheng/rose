#include <stdio.h>

#include "tiny_lexer.h"
#include "messaging-framework.h"

size_t 
messaging_framework_receive_into_buffer(QUEX_TYPE_CHARACTER*, size_t);

int 
main(int argc, char** argv) 
{        
    quex_Token       token;
    quex_tiny_lexer  qlex;
    size_t           receive_n = (size_t)-1;
    size_t           BufferSize = 1024;
    char             buffer[1024];
    bool             out_f = false;

    QUEX_NAME(construct_memory)(&qlex, 0x0, 0, 0x0, 0x0, false);
    QUEX_NAME_TOKEN(construct)(&token);

    (void)QUEX_NAME(token_p_switch)(&qlex, &token);
    while( 1 + 1 == 2 ) {
        /* -- Initialize the filling of the fill region         */
        QUEX_NAME(buffer_fill_region_prepare)(&qlex);

        /* -- Call the low lever driver to fill the fill region */
        receive_n = messaging_framework_receive_into_buffer_syntax_chunk(QUEX_NAME(buffer_fill_region_begin)(&qlex), 
                                                                         QUEX_NAME(buffer_fill_region_size)(&qlex));

        /* -- Inform the buffer about the number of loaded characters NOT NUMBER OF BYTES! */
        QUEX_NAME(buffer_fill_region_finish)(&qlex, receive_n);

        /* -- Loop until the 'termination' token arrives */
        while( 1 + 1 == 2 ) {
            const QUEX_TYPE_TOKEN_ID TokenID = QUEX_NAME(receive)(&qlex);

            if( TokenID == QUEX_TKN_TERMINATION ) break;
            if( TokenID == QUEX_TKN_BYE )         { out_f = true; break; }

            printf("Consider: %s \n", QUEX_NAME_TOKEN(get_string)(&token, buffer, BufferSize));
        }
        if( out_f ) break;
    }

    QUEX_NAME(destruct)(&qlex);
    QUEX_NAME_TOKEN(destruct)(&token);
    return 0;
}

