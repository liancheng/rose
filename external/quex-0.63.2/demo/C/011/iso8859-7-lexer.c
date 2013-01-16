#include <stdio.h> 

// (*) include lexical analyser header
#include "ISO8859_7_Lex.h"

int 
main(int argc, char** argv) 
{        
    quex_Token*        token_p;
    quex_ISO8859_7_Lex qlex;
    size_t             BufferSize = 1024;
    char               buffer[1024];
    
    QUEX_NAME(construct_file_name)(&qlex, "example-iso8859-7.txt", 0x0, false);

    // (*) loop until the 'termination' token arrives
    do {
        // (*) get next token from the token stream
        QUEX_NAME(receive)(&qlex, &token_p);

        /* (*) print out token information
         *     'get_string' automagically converts codec bytes into utf8 */
        printf("%s \n", QUEX_NAME_TOKEN(get_string)(token_p, buffer, BufferSize));
#       if 0
        cout << "\t\t plain bytes: ";
        for(QUEX_TYPE_CHARACTER* iterator = (uint8_t*)tmp.c_str(); *iterator ; ++iterator) {
            printf("%02X.", (int)*iterator);
        }
#       endif

        // (*) check against 'termination'
    } while( token_p->_id != TKN_TERMINATION );

    QUEX_NAME(destruct)(&qlex);
    return 0;
}
