#include<cstdio> 

// (*) include lexical analyser header
#include "UTF16Lex"

using namespace std;

int 
main(int argc, char** argv) 
{        
    using namespace quex;

    if( argc == 1 ) {
       printf("Required at least one argument: 'LE' or 'BE'.\n");
       return -1;
    }

    Token*       token;
    bool         BigEndianF = (strcmp(argv[1], "BE") == 0); 
    const char*  file_name = BigEndianF ? "example-utf16be.txt" : "example-utf16le.txt";
   
    /* NOTE: On a big endian machine (e.g. PowerPC) the byte reversion flag
     *       might be set to 'not BigEndianF'                                */
    UTF16Lex     qlex(file_name, 0x0, /* Byte Order Reversion */ BigEndianF);

    printf("## input file           = %s\n", file_name);
    printf("## byte order reversion = %s\n", qlex.byte_order_reversion() ? "true" : "false");
    
    do {
        qlex.receive(&token);

        /* Print the lexeme in utf8 format. */
        printf("%s\n", (char*)(string(*token).c_str()));

        // (*) check against 'termination'
    } while( token->type_id() != TKN_TERMINATION );

    return 0;
}
