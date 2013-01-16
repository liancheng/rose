#include<fstream>    
#include<iostream> 
#include<sstream> 

#include "tiny_lexer"
#include "messaging-framework.h"

size_t 
messaging_framework_receive_into_buffer(QUEX_TYPE_CHARACTER*, size_t);

int 
main(int argc, char** argv) 
{        
    using namespace std;

    quex::Token       token;
    quex::tiny_lexer  qlex((QUEX_TYPE_CHARACTER*)0x0, 0); /* No args to constructor --> raw memory */

    (void)qlex.token_p_switch(&token);
    while( 1 + 1 == 2 ) {
        // -- Initialize the filling of the fill region
        qlex.buffer_fill_region_prepare();

        // -- Call the low lever driver to fill the fill region
        size_t receive_n = messaging_framework_receive_into_buffer_syntax_chunk(qlex.buffer_fill_region_begin(), 
                                                                                qlex.buffer_fill_region_size());

        // -- Inform the buffer about the number of loaded characters NOT NUMBER OF BYTES!
        qlex.buffer_fill_region_finish(receive_n);

        // -- Loop until the 'termination' token arrives
        while( 1 + 1 == 2 ) {
            const QUEX_TYPE_TOKEN_ID TokenID = qlex.receive();

            if( TokenID == QUEX_TKN_TERMINATION ) break;
            if( TokenID == QUEX_TKN_BYE )         return 0;

            cout << "Consider: " << string(token) << endl;
        }
    }

    return 0;
}

