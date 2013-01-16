#include <stdio.h>

#include "tiny_lexer_utf8.h"
#include "messaging-framework.h"

typedef struct {
    uint8_t* begin;
    uint8_t* end;
} MemoryChunk;

int 
main(int argc, char** argv) 
{        
    quex_tiny_lexer_utf8    qlex;   

    quex_Token         token_bank[2];     /* Two tokens required, one for look-ahead */
    quex_Token*        prev_token;        /* Use pointers to swap quickly.           */

    uint8_t*           rx_buffer = 0x0;   /* A pointer to the receive buffer that
    *                                      * the messaging framework provides.       */

    MemoryChunk        chunk;             /* Pointers to the memory positions under  
    *                                      * consideration.                          */
    size_t             BufferSize = 1024;
    char               buffer[1024];
    size_t             size = (size_t)-1;
    QUEX_TYPE_TOKEN_ID token_id = 0;

    QUEX_TYPE_CHARACTER*  prev_lexeme_start_p = 0x0; /* Store the start of the  
    *                                                 * lexeme for possible backup.  */

    QUEX_NAME(construct_memory)(&qlex, (QUEX_TYPE_CHARACTER*)0x0, 0, (QUEX_TYPE_CHARACTER*)0x0, "UTF-8", false);

    /* -- initialize the token pointers */
    QUEX_NAME_TOKEN(construct)(&token_bank[0]);
    QUEX_NAME_TOKEN(construct)(&token_bank[1]);
    token_bank[0]._id = QUEX_TKN_TERMINATION;

    prev_token = &(token_bank[1]);

    QUEX_NAME(token_p_switch)(&qlex, &token_bank[0]);

    /* -- trigger reload of memory */
    chunk.begin = chunk.end;

    /* -- LOOP until 'bye' token arrives */
    while( 1 + 1 == 2 ) {
        /* -- Receive content from a messaging framework */
        /*    The function 'buffer_fill_region_append()' may possibly not */
        /*    concatinate all content, so it needs to be tested wether new */
        /*    content needs to be loaded. */
        if( chunk.begin == chunk.end ) {
            /* -- If the receive buffer has been read, it can be released. */
            if( rx_buffer != 0x0 ) messaging_framework_release(rx_buffer);
            /* -- Setup the pointers  */
            size  = messaging_framework_receive(&rx_buffer);
            chunk.begin = rx_buffer;
            chunk.end   = chunk.begin + size;
        } else {
            /* If chunk.begin != chunk.end, this means that there are still */
            /* some characters in the pipeline. Let us use them first. */
        }

        /* -- Copy buffer content into the analyzer's buffer */
        /*    (May be, not all content can be copied into the buffer, in  */
        /*     this case the '_append(...)' function returns a position */
        /*     different from 'chunk.end'. This would indicate the there */
        /*     are still bytes left. The next call of '_apend(...)' will */
        /*     deal with it.) */
        chunk.begin = (uint8_t*)QUEX_NAME(buffer_fill_region_append_conversion)(&qlex, chunk.begin, chunk.end);

        /* -- Loop until the 'termination' token arrives */
        token_id = 0;
        while( 1 + 1 == 2 ) {
            prev_lexeme_start_p = QUEX_NAME(buffer_lexeme_start_pointer_get)(&qlex);
            
            /* Let the previous token be the current token of the previous run. */
            prev_token = QUEX_NAME(token_p_switch)(&qlex, prev_token);

            token_id = QUEX_NAME(receive)(&qlex);

            /* TERMINATION => possible reload */
            /* BYE         => end of game */
            if( token_id == QUEX_TKN_TERMINATION )
                break;

            /* If the previous token was not a TERMINATION, it can be considered */
            /* by the syntactical analyzer (parser). */
            if( prev_token->_id != QUEX_TKN_TERMINATION )
                printf("Consider: %s \n", QUEX_NAME_TOKEN(get_string)(prev_token, buffer, BufferSize));

            if( token_id == QUEX_TKN_BYE ) 
                break;
        }

        /* -- If the 'bye' token appeared, leave! */
        if( token_id == QUEX_TKN_BYE ) break;

        /* -- Reset the input pointer, so that the last lexeme before TERMINATION */
        /*    enters the matching game again. */
        QUEX_NAME(buffer_input_pointer_set)(&qlex, prev_lexeme_start_p);
    }
    printf("Consider: %s \n", QUEX_NAME_TOKEN(get_string)(prev_token, buffer, BufferSize));

    QUEX_NAME(destruct)(&qlex);
    QUEX_NAME_TOKEN(destruct)(&token_bank[0]);
    QUEX_NAME_TOKEN(destruct)(&token_bank[1]);
    return 0;
}

