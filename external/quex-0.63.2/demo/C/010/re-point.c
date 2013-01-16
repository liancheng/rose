#include<stdio.h>    
#include<string.h> 

#include "tiny_lexer.h"
#include "messaging-framework.h"

void test(quex_tiny_lexer* qlex);

int 
main(int argc, char** argv) 
{        
    quex_tiny_lexer      qlex;
    QUEX_TYPE_CHARACTER* remainder = 0x0;

    QUEX_NAME(construct_memory)(&qlex, 0x0, 0, (QUEX_TYPE_CHARACTER*)0x0, 0x0, false);

    /* In this example we do the same as in 'point.cpp'
     * -- only that the use different buffers for each run.
     *    This requires a 'reset_buffer' call, as shown below. */
    test(&qlex);
    test(&qlex);
    test(&qlex);

    /* Delete remaining memory buffer that is still inside the analyzer */
    remainder = QUEX_NAME(reset_buffer)(&qlex, 0x0, 0, 0x0, 0x0);
    if( remainder != 0x0 ) free(remainder);

    QUEX_NAME(destruct)(&qlex);
    return 0;
}

void
get_new_memory_to_analyze(QUEX_TYPE_CHARACTER** buffer, size_t* buffer_size)
{
    /* Get some chunk of memory */
    QUEX_TYPE_CHARACTER*  new_memory = (QUEX_TYPE_CHARACTER*)malloc(sizeof(QUEX_TYPE_CHARACTER)*MESSAGING_FRAMEWORK_BUFFER_SIZE);

    /* Call the low lever driver to fill the fill region */
    *buffer_size = messaging_framework_receive_to_internal_buffer() + 1;

    /* Copy the content from the messaging buffer to the provided memory
     * so that the analyzer has something to chew on.                    */
    memcpy(new_memory, MESSAGING_FRAMEWORK_BUFFER, (*buffer_size) * sizeof(QUEX_TYPE_CHARACTER));

    *buffer = new_memory;
}

void 
test(quex_tiny_lexer* qlex)
{
    QUEX_TYPE_TOKEN       token;           
    QUEX_TYPE_CHARACTER*  buffer      = 0x0;
    size_t                buffer_size = 0;
    size_t                UTF8BufferSize = 1024;
    char                  utf8_buffer[1024];
    QUEX_TYPE_CHARACTER*  prev_memory = 0x0;

    get_new_memory_to_analyze(&buffer, &buffer_size);

    /* Setup the memory to be analyzed (this is the 're-point' operation). 
     * (buffer is one character larger than the content, so that it can contain the
     *  buffer limit code at the end.)                                              */
    prev_memory = QUEX_NAME(reset_buffer)(qlex, buffer, buffer_size, 
                                          /* End of Content */ buffer + buffer_size - 1, 0x0); 
    /* If there was some old memory, than delete it. */
    if( prev_memory != 0x0 ) free(prev_memory);

    /* QUEX_NAME(Buffer_show_byte_content)(&qlex->buffer, 5); */

    /* -- Loop until the 'termination' token arrives */
    QUEX_NAME(token_p_switch)(qlex, &token);
    do {
        QUEX_NAME(receive)(qlex);

        if( token._id != QUEX_TKN_TERMINATION )
            printf("Consider: %s \n", QUEX_NAME_TOKEN(get_string)(&token, utf8_buffer, UTF8BufferSize));

        if( token._id == QUEX_TKN_BYE ) 
            printf("##\n");
        
    } while( token._id != QUEX_TKN_TERMINATION );

    QUEX_NAME_TOKEN(destruct)(&token);

    printf("<<End of Run>>\n");
}

