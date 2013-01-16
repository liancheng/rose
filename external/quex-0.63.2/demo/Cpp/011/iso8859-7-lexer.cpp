#include<cstdio> 

// (*) include lexical analyser header
#include "ISO8859_7_Lex"
#include "ISO8859_7_Lex-converter-iso8859_7"

using namespace std;

int 
main(int argc, char** argv) 
{        
    using namespace quex;

    Token*           token_p;
    Token::__string  tmp;
    ISO8859_7_Lex    qlex("example-iso8859-7.txt");
    

    // (*) loop until the 'termination' token arrives
    do {
        // (*) get next token from the token stream
        qlex.receive(&token_p);

        // (*) print out token information
        cout << string(*token_p) << endl;
#       if 0
        cout << "\t\t plain bytes: ";
        for(QUEX_TYPE_CHARACTER* iterator = (uint8_t*)tmp.c_str(); *iterator ; ++iterator) {
            printf("%02X.", (int)*iterator);
        }
#       endif

        // (*) check against 'termination'
    } while( token_p->type_id() != TKN_TERMINATION );

    return 0;
}
