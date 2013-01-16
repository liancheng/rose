#include<fstream>    
#include<iostream> 
#include<sstream> 

#include "tiny_lexer"
#include "messaging-framework.h"

typedef struct {
    QUEX_TYPE_CHARACTER* begin;
    QUEX_TYPE_CHARACTER* end;
} MemoryChunk;

int 
main(int argc, char** argv) 
{        
    using namespace std;

    // Zero pointer to constructor --> memory managed by user
    quex::tiny_lexer      qlex((QUEX_TYPE_CHARACTER*)0x0, 0);   
    quex::Token           token;           
    QUEX_TYPE_CHARACTER*  rx_buffer = 0x0; // receive buffer
    MemoryChunk           chunk;

    // -- trigger reload of memory
    chunk.begin = chunk.end;

    // -- LOOP until 'bye' token arrives
    (void)qlex.token_p_switch(&token);
    while( 1 + 1 == 2 ) {
        // -- Receive content from a messaging framework
        //    The function 'buffer_fill_region_append()' may possibly not
        //    concatinate all content, so it needs to be tested wether new
        //    content needs to be loaded.
        if( chunk.begin == chunk.end ) {
            // -- If the receive buffer has been read, it can be released.
            if( rx_buffer != 0x0 ) messaging_framework_release(rx_buffer);
            // -- Setup the pointers 
            const size_t Size  = messaging_framework_receive_syntax_chunk(&rx_buffer);
            chunk.begin = rx_buffer;
            chunk.end   = chunk.begin + Size;
        } else {
            // If chunk.begin != chunk.end, this means that there are still
            // some characters in the pipeline. Let us use them first.
        }

        // -- Copy buffer content into the analyzer's buffer
        //    (May be, not all content can be copied into the buffer, in 
        //     this case the '_append(...)' function returns a position
        //     different from 'chunk.end'. This would indicate the there
        //     are still bytes left. The next call of '_apend(...)' will
        //     deal with it.)
        chunk.begin = (uint8_t*)qlex.buffer_fill_region_append(chunk.begin, chunk.end);

        // -- Loop until the 'termination' token arrives
        while( 1 + 1 == 2 ) {
            const QUEX_TYPE_TOKEN_ID TokenID = qlex.receive();

            // TERMINATION => possible reload
            // BYE         => end of game
            if( TokenID == QUEX_TKN_TERMINATION ) break;
            if( TokenID == QUEX_TKN_BYE )         return 0;

            cout << "Consider: " << string(token) << endl;
        }
    }

    return 0;
}

