#include<fstream>    
#include<iostream> 
#include<sstream> 

#include <./tiny_lexer_st>

int 
main(int argc, char** argv) 
{        
    using namespace std;

    quex::Token          token;
    // Zero pointer to constructor --> use raw memory
    quex::tiny_lexer_st  qlex((QUEX_TYPE_CHARACTER*)0x0, 0);   

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";
    cout << "Please, type an arbitrary sequence of the following:\n";
    cout << "-- One of the words: 'hello', 'world', 'hallo', 'welt', 'bonjour', 'le monde'.\n";
    cout << "-- An integer number.\n";
    cout << "-- The word 'bye' in order to terminate.\n";
    cout << "Please, terminate each line with pressing [enter].\n";

    int number_of_tokens = 0;
    (void)qlex.token_p_switch(&token);
    while( cin ) {
        qlex.buffer_fill_region_prepare();
        
        // Read a line from standard input
        cin.getline((char*)qlex.buffer_fill_region_begin(), 
                    qlex.buffer_fill_region_size());
        cout << "[[Received " << cin.gcount() << " characters in line.]]\n";
        
        if( cin.gcount() == 0 ) {
            return 0;
        }
        // Inform about number of read characters. Note, that getline
        // writes a terminating zero, which has not to be part of the 
        // buffer content.
        qlex.buffer_fill_region_finish(cin.gcount() - 1);
        
        // Loop until the 'termination' token arrives
        do {
            (void)qlex.receive();
            cout << string(token) << endl;
            ++number_of_tokens;
        } while( token.type_id() != QUEX_TKN_TERMINATION && token.type_id() != QUEX_TKN_BYE );
        
        cout << "[[End of Input]]\n";

        if( token.type_id() == QUEX_TKN_BYE ) break;
    }

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    return 0;
}

