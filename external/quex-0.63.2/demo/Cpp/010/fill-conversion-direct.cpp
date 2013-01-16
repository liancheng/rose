#include<fstream>    
#include<iostream> 
#include<sstream> 

#include "tiny_lexer_utf8"
#include "messaging-framework.h"

int 
main(int argc, char** argv) 
{        
    using namespace std;

    quex::Token           token_bank[2];
    quex::Token*          prev_token;
    // Zero pointer to constructor --> use raw memory
    quex::tiny_lexer_utf8 qlex((QUEX_TYPE_CHARACTER*)0x0, 0, (QUEX_TYPE_CHARACTER*)0x0, "UTF-8");   
    QUEX_TYPE_CHARACTER*  prev_lexeme_start_p = 0x0;

    // -- initialize the token pointers
    prev_token = &(token_bank[1]);
    token_bank[0].set(QUEX_TKN_TERMINATION);
    qlex.token_p_switch(&token_bank[0]);

    while( 1 + 1 == 2 ) {
        // -- Initialize the filling of the fill region
        qlex.buffer_conversion_fill_region_prepare();

        // -- Call the low lever driver to fill the fill region
        size_t receive_n = messaging_framework_receive_into_buffer(
                                         qlex.buffer_conversion_fill_region_begin(), 
                                         qlex.buffer_conversion_fill_region_size());

        // -- Inform the buffer about the number of loaded characters NOT NUMBER OF BYTES!
        qlex.buffer_conversion_fill_region_finish(receive_n);

        // -- Loop until the 'termination' token arrives
        QUEX_TYPE_TOKEN_ID  token_id = 0;
        while( 1 + 1 == 2 ) {
            prev_lexeme_start_p = qlex.buffer_lexeme_start_pointer_get();
            
            // Let the previous token be the current token of the previous run.
            prev_token = qlex.token_p_switch(prev_token);

            token_id = qlex.receive();
            if( token_id == QUEX_TKN_TERMINATION || token_id == QUEX_TKN_BYE )
                break;
            if( prev_token->type_id() != QUEX_TKN_TERMINATION ) 
                cout << "Consider: " << string(*prev_token) << endl;
        }

        if( token_id == QUEX_TKN_BYE ) break;

        qlex.buffer_input_pointer_set(prev_lexeme_start_p);
    }
    cout << "Consider: " << string(*prev_token) << endl;

    return 0;
}

