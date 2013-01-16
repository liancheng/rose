#include <stdio.h>
#include <./tiny_lexer_st.h>

int 
main(int argc, char** argv) 
{        
    quex_Token           token;
    quex_tiny_lexer_st   qlex;   
    size_t               received_n = (size_t)-1;
    size_t               BufferSize = 1024;
    char                 buffer[1024];
    char*   qlex_buffer      = 0x0;
    size_t  qlex_buffer_size = 0;

    QUEX_NAME_TOKEN(construct)(&token);
    /* Zero pointer to constructor --> use raw memory */
    QUEX_NAME(construct_memory)(&qlex, 0x0, 0, 0x0, 0x0, false);

    printf("Please, type an arbitrary sequence of the following:\n");
    printf("-- One of the words: 'hello', 'world', 'hallo', 'welt', 'bonjour', 'le monde'.\n");
    printf("-- An integer number.\n");
    printf("-- The word 'bye' in order to terminate.\n");
    printf("Please, terminate each line with pressing [enter].\n");

    (void)QUEX_NAME(token_p_switch)(&qlex, &token);
    while( received_n ) {
        QUEX_NAME(buffer_fill_region_prepare)(&qlex);
        
        /* Read a line from standard input */
        qlex_buffer      = (char*)QUEX_NAME(buffer_fill_region_begin)(&qlex);
        qlex_buffer_size = QUEX_NAME(buffer_fill_region_size(&qlex));
        while( fgets(qlex_buffer, qlex_buffer_size, stdin) == NULL );
        received_n = strlen(qlex_buffer);
        printf("[[Received %i characters in line.]]\n", (int)received_n);
        
        if( received_n == 0 ) {
            break;
        }
        /* Inform about number of read characters. */
        QUEX_NAME(buffer_fill_region_finish)(&qlex, received_n - 1);
        
        /* Loop until the 'termination' token arrives */
        do {
            QUEX_NAME(receive)(&qlex);
            printf("%s \n", QUEX_NAME_TOKEN(get_string)(&token, buffer, BufferSize));
        } while( token._id != QUEX_TKN_TERMINATION && token._id != QUEX_TKN_BYE );
        
        printf("[[End of Input]]\n");

        if( token._id == QUEX_TKN_BYE ) break;
    }

    QUEX_NAME_TOKEN(destruct)(&token);
    /* Zero pointer to constructor --> use raw memory */
    QUEX_NAME(destruct)(&qlex);
    return 0;
}

