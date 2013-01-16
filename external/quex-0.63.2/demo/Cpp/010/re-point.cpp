#include<fstream>    
#include<iostream> 
#include<sstream> 
#include<cstring> 

#include "tiny_lexer"
#include "messaging-framework.h"

void test(quex::tiny_lexer& qlex);

int 
main(int argc, char** argv) 
{        
    using namespace std;

    quex::tiny_lexer   qlex((QUEX_TYPE_CHARACTER*)0x0, 0, (QUEX_TYPE_CHARACTER*)0x0);

    /* In this example we do the same as in 'point.cpp'
     * -- only that the use different buffers for each run.
     *    This requires a 'reset_buffer' call, as shown below. */
    test(qlex);
    test(qlex);
    test(qlex);

    /* Delete remaining memory buffer that is still inside the analyzer */
    QUEX_TYPE_CHARACTER* remainder = qlex.reset_buffer(0x0, 0, 0x0);
    if( remainder != 0x0 ) delete [] remainder;

    return 0;
}

void
get_new_memory_to_analyze(QUEX_TYPE_CHARACTER** buffer, size_t* buffer_size)
{
    /* Get some chunk of memory */
    QUEX_TYPE_CHARACTER*  new_memory = new QUEX_TYPE_CHARACTER[MESSAGING_FRAMEWORK_BUFFER_SIZE];

    /* Call the low lever driver to fill the fill region */
    *buffer_size = messaging_framework_receive_to_internal_buffer() + 1;

    /* Copy the content from the messaging buffer to the provided memory
     * so that the analyzer has something to chew on.                    */
    memcpy(new_memory, MESSAGING_FRAMEWORK_BUFFER, (*buffer_size) * sizeof(QUEX_TYPE_CHARACTER));

    *buffer = new_memory;
}

void 
test(quex::tiny_lexer& qlex)
{
    using namespace std;

    quex::Token          token;           
    QUEX_TYPE_CHARACTER* buffer      = 0x0;
    size_t               buffer_size = 0;
    
    get_new_memory_to_analyze(&buffer, &buffer_size);

    /* Setup the memory to be analyzed (this is the 're-point' operation). 
     * (buffer is one character larger than the content, so that it can contain the
     *  buffer limit code at the end.)                                              */
    QUEX_TYPE_CHARACTER* prev_memory = qlex.reset_buffer(buffer, buffer_size, 
                                                         /* End of Content */ buffer + buffer_size - 1); 
    /* If there was some old memory, than delete it. */
    if( prev_memory != 0x0 ) delete [] prev_memory;

    // QUEX_NAME(Buffer_show_byte_content)(&qlex.buffer, 5);

    // -- Loop until the 'termination' token arrives
    (void)qlex.token_p_switch(&token);
    do {
        qlex.receive();

        if( token.type_id() != QUEX_TKN_TERMINATION )
            cout << "Consider: " << string(token) << endl;

        if( token.type_id() == QUEX_TKN_BYE ) 
            cout << "##\n";

    } while( token.type_id() != QUEX_TKN_TERMINATION );

    cout << "<<End of Run>>\n";
}

