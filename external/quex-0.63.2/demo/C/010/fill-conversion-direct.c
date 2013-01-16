#include <stdio.h>

#include "tiny_lexer_utf8.h"
#include "messaging-framework.h"

int 
main(int argc, char** argv) 
{        
    quex_Token            token_bank[2];
    quex_Token*           prev_token;
    quex_tiny_lexer_utf8  qlex;
    QUEX_TYPE_CHARACTER*  prev_lexeme_start_p = 0x0;
    size_t                BufferSize = 1024;
    char                  buffer[1024];
    size_t                receive_n = (size_t)-1;
    QUEX_TYPE_TOKEN_ID    token_id = 0;
 
    QUEX_NAME(construct_memory)(&qlex, 0x0, 0, 0x0, "UTF-8", false);

    /* -- initialize the token pointers */
    QUEX_NAME_TOKEN(construct)(&token_bank[0]);
    QUEX_NAME_TOKEN(construct)(&token_bank[1]);
    token_bank[0]._id = QUEX_TKN_TERMINATION;

    prev_token = &(token_bank[1]);

    QUEX_NAME(token_p_switch)(&qlex, &token_bank[0]);

    while( 1 + 1 == 2 ) {
        /* -- Initialize the filling of the fill region         */
        QUEX_NAME(buffer_conversion_fill_region_prepare)(&qlex);

        /* -- Call the low lever driver to fill the fill region */
        receive_n = messaging_framework_receive_into_buffer(
                                  QUEX_NAME(buffer_conversion_fill_region_begin)(&qlex), 
                                  QUEX_NAME(buffer_conversion_fill_region_size)(&qlex));

        /* -- Inform the buffer about the number of loaded characters NOT NUMBER OF BYTES! */
        QUEX_NAME(buffer_conversion_fill_region_finish)(&qlex, receive_n);

        /* -- Loop until the 'termination' token arrives */
        token_id = 0;
        while( 1 + 1 == 2 ) {
            prev_lexeme_start_p = QUEX_NAME(buffer_lexeme_start_pointer_get)(&qlex);
            
            /* Let the previous token be the current token of the previous run. */
            prev_token = QUEX_NAME(token_p_switch)(&qlex, prev_token);

            token_id = QUEX_NAME(receive)(&qlex);
            if( token_id == QUEX_TKN_TERMINATION || token_id == QUEX_TKN_BYE )
                break;
            if( prev_token->_id != QUEX_TKN_TERMINATION ) 
                printf("Consider: %s \n", QUEX_NAME_TOKEN(get_string)(prev_token, buffer, BufferSize));
        }

        if( token_id == QUEX_TKN_BYE ) break;

        QUEX_NAME(buffer_input_pointer_set)(&qlex, prev_lexeme_start_p);
    }
    printf("Consider: %s \n", QUEX_NAME_TOKEN(get_string)(prev_token, buffer, BufferSize));

    QUEX_NAME(destruct)(&qlex);
    QUEX_NAME_TOKEN(destruct)(&token_bank[0]);
    QUEX_NAME_TOKEN(destruct)(&token_bank[1]);
    return 0;
}

