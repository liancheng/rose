#include <stdio.h> 

// (*) include lexical analyser header
#include "UTF8Lex.h"

int 
main(int argc, char** argv) 
{        
    quex_Token*   token_p = 0x0;
    size_t        BufferSize = 1024;
    char          buffer[1024];
    quex_UTF8Lex  qlex;
    
    QUEX_NAME(construct_file_name)(&qlex, "example-utf8.txt", 0x0, false);

    // (*) loop until the 'termination' token arrives
    do {
        // (*) get next token from the token stream
        QUEX_NAME(receive)(&qlex, &token_p);

        /* (*) print out token information
         *     'get_string' automagically converts codec bytes into utf8 */
        printf("%s \n", QUEX_NAME_TOKEN(get_string)(token_p, buffer, BufferSize));

        // (*) check against 'termination'
    } while( token_p->_id != TKN_TERMINATION );

    QUEX_NAME(destruct)(&qlex);
    return 0;
}
