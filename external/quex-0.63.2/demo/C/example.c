#include <stdio.h>    

#include "EasyLexer.h"


#ifndef     ENCODING_NAME
#    define ENCODING_NAME (0x0)
#endif

int 
main(int argc, char** argv) 
{        
    quex_Token*     token_p = 0x0;
    size_t          number_of_tokens = 0;
    quex_EasyLexer  qlex;
#   ifdef PRINT_TOKEN
    const size_t    BufferSize = 1024;
    char            buffer[1024];
#   endif
    const char*     FileName = (argc == 1) ? "example.txt" : argv[1];

    quex_EasyLexer_construct_file_name(&qlex, FileName, ENCODING_NAME, false);
    /* Alternatives:
     * QUEX_NAME(construct_memory)(&qlex, MemoryBegin, MemorySize,
     *                             CharacterEncodingName (default 0x0),
     *                             ByteOrderReversionF   (default false));
     * QUEX_NAME(construct_FILE)(&qlex, FILE_handle, 
     *                           CharacterEncodingName (default 0x0),
     *                           ByteOrderReversionF   (default false)); */
    printf(",-----------------------------------------------------------------\n");
    printf("| [START]\n");

    /* Loop until the 'termination' token arrives */
    do {
        /* Get next token from the token stream   */
        quex_EasyLexer_receive(&qlex, &token_p);

        /* Print out token information            */
#       ifdef PRINT_LINE_COLUMN_NUMBER
        printf("(%i, %i)  \t", (int)token_p->_line_n, (int)token_p->_column_n);
#       endif
#       ifdef PRINT_TOKEN
        printf("%s \n", QUEX_NAME_TOKEN(get_string)(token_p, buffer, BufferSize));
#       else
        printf("%s\n", QUEX_NAME_TOKEN(map_id_to_name)(token_p->_id));
#       endif

#       ifdef SPECIAL_ACTION
        SPECIAL_ACTION(&qlex, &my_token);
#       endif
        ++number_of_tokens;

        /* Check against 'termination'            */
    } while( token_p->_id != QUEX_TKN_TERMINATION );

    printf("| [END] number of token = %i\n", number_of_tokens);
    printf("`-----------------------------------------------------------------\n");

    QUEX_NAME(destruct)(&qlex);

    return 0;
}
