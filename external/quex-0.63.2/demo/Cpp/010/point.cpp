#include <fstream>
#include <iostream> 
#include <sstream> 

#include "tiny_lexer"
#include "messaging-framework.h"

int 
main(int argc, char** argv) 
{        
    using namespace std;

    quex::Token        token;           
    quex::tiny_lexer   qlex(MESSAGING_FRAMEWORK_BUFFER, 
                            MESSAGING_FRAMEWORK_BUFFER_SIZE,
                            MESSAGING_FRAMEWORK_BUFFER + 1); 
    size_t             receive_n = (size_t)-1;
    int                i = 0;

    if( QUEX_SETTING_BUFFER_MIN_FALLBACK_N != 0 ) {
        QUEX_ERROR_EXIT("This method fails if QUEX_SETTING_BUFFER_MIN_FALLBACK_N != 0\n"
                        "Consider using the method described in 're-point.c'.");
    }

    /* Iterate 3 times doing the same thing in order to illustrate
     * the repeated activation of the same chunk of memory. */
    for(i = 0; i < 3; ++i ) {
        qlex.buffer_fill_region_prepare();

        /* -- Call the low lever driver to fill the fill region */
        receive_n = messaging_framework_receive_to_internal_buffer();

        /* -- Inform the buffer about the number of loaded characters NOT NUMBER OF BYTES! */
        qlex.buffer_fill_region_finish(receive_n-1);
        /* QUEX_NAME(Buffer_show_byte_content)(&qlex.buffer, 5); */

        /* -- Loop until the 'termination' token arrives */
        (void)qlex.token_p_switch(&token);
        do {
            qlex.receive();

            if( token.type_id() != QUEX_TKN_TERMINATION )
                printf("Consider: %s\n", string(token).c_str());

            if( token.type_id() == QUEX_TKN_BYE ) 
                printf("##\n");

        } while( token.type_id() != QUEX_TKN_TERMINATION );
    }

    return 0;
}

