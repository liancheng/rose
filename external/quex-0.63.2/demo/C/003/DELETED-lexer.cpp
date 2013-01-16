#include<fstream>    
#include<iostream> 

// (*) include lexical analyser header
#include <./tiny_lexer>

using namespace std;

int 
main(int argc, char** argv) 
{        
    // (*) create token
    quex::Token        Token;
    // (*) create the lexical analyser
    //     if no command line argument is specified user file 'example.txt'
    quex::tiny_lexer   qlex(argc == 1 ? "example.txt" : argv[1], "UTF-8");

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";

    int number_of_tokens = 0;
    // (*) loop until the 'termination' token arrives
    do {
        // (*) get next token from the token stream
        qlex.receive(&Token);

        // (*) print out token information
        //     -- line number and column number
        // cout << "(" << qlex.line_number() << ", " << qlex.column_number() << ")  \t";
        //     -- name of the token

        /* Caveat: Do not use 'Token.text().c_str()'. The content of 'text()' is an 
         *         array of type QUEX_TYPE_TOKEN which is a multibyte type (if -b 2 or
         *         -b 3 was passed as command line argument to quex.). Requesting
         *         '.c_str()' only returns a pointer to the multi-byte sequence but
         *         not to a UTF8 converted text.                                          
         *
         *         If only the text is requested, please use '.utf8_text()'.            */   
        cout << string(Token);
        cout << endl;

        ++number_of_tokens;

        // (*) check against 'termination'
    } while( Token.type_id() != QUEX_TKN_TERMINATION );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    return 0;
}
