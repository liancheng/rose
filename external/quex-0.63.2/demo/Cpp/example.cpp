#include<fstream>    
#include<iostream> 

// (*) include lexical analyser header
#include "EasyLexer"


#ifndef     ENCODING_NAME
#    define ENCODING_NAME (0x0)
#endif

int 
main(int argc, char** argv) 
{        
    using namespace std;

    quex::Token*       token_p = 0x0;
    // (*) create the lexical analyser
    //     1st arg: input file, default = 'example.txt'
    //     2nd arg: input character encoding name, 0x0 --> no codec conversion
    quex::EasyLexer    qlex(argc == 1 ? "example.txt" : argv[1], ENCODING_NAME);

    cout << ",-----------------------------------------------------------------\n";
    cout << "| [START]\n";

    int number_of_tokens = 0;
    // (*) loop until the 'termination' token arrives
    do {
        // (*) get next token from the token stream
        qlex.receive(&token_p);

        // (*) print out token information
#       ifdef PRINT_LINE_COLUMN_NUMBER
        cout << "(" << token_p->line_number() << ", " << token_p->column_number() << ")  \t";
#       endif
#       ifdef PRINT_TOKEN
        cout << string(*token_p) << endl;
#       else
        cout << token_p->type_id_name() << endl;
#       endif

#       ifdef SPECIAL_ACTION
        SPECIAL_ACTION(&qlex, &my_token);
#       endif

        ++number_of_tokens;

        // (*) check against 'termination'
    } while( token_p->type_id() != QUEX_TKN_TERMINATION );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`-----------------------------------------------------------------\n";

    return 0;
}
