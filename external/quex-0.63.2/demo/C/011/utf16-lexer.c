#include <stdio.h> 

// (*) include lexical analyser header
#include "UTF16Lex.h"

int 
main(int argc, char** argv) 
{        
    quex_Token*           token_p    = 0x0;
    bool                  BigEndianF = (argc < 2 || (strcmp(argv[1], "BE") == 0)); 
    const char*           file_name  = BigEndianF ? "example-utf16be.txt" : "example-utf16le.txt";
    quex_UTF16Lex         qlex;
    size_t                BufferSize = 1024;
    char                  buffer[1024];

    if( argc == 1 ) {
        printf("Required at least one argument: 'LE' or 'BE'.\n");
        return -1;
    }

   
    /* NOTE: On a big endian machine (e.g. PowerPC) the byte reversion flag
     *       might be set to 'not BigEndianF'                                */

    QUEX_NAME(construct_file_name)(&qlex, file_name, 0x0, BigEndianF);

    printf("## input file           = %s\n", file_name);
    printf("## byte order reversion = %s\n", QUEX_NAME(byte_order_reversion)(&qlex) ? "true" : "false");
    
    do {
        QUEX_NAME(receive)(&qlex, &token_p);

        /* Print the lexeme in utf8 format. */
        printf("%s \n", QUEX_NAME_TOKEN(get_string)(token_p, buffer, BufferSize));

        // (*) check against 'termination'
    } while( token_p->_id != TKN_TERMINATION );

    QUEX_NAME(destruct)(&qlex);

    return 0;
}
