#include<fstream>    
#include<iostream> 

// (*) include lexical analyser header
#include "EasyLexer"
#include "EasyLexer-token_ids"

using namespace std;

QUEX_TYPE_CHARACTER  EmptyLexeme = 0x0000;  /* Only the terminating zero */

int 
main(int argc, char** argv) 
{        
    using namespace quex;

    // (*) create token
    Token*     token;
    // (*) create the lexical analyser
    //     if no command line argument is specified user file 'example.txt'
    EasyLexer  qlex(argc == 1 ? "example.txt" : argv[1]);

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";

    int  number_of_tokens = 0;
    bool continue_lexing_f = true;
    // (*) loop until the 'termination' token arrives
    token = qlex.token_p();
    do {
        // (*) get next token from the token stream
        QUEX_TYPE_TOKEN_ID token_id = qlex.receive();

        // (*) print out token information
        print_token(&qlex, token, true);

        if( token_id == QUEX_TKN_INCLUDE ) { 
            token_id = qlex.receive();
            print_token(&qlex, token, true);
            if( token_id != QUEX_TKN_IDENTIFIER ) {
                continue_lexing_f = false;
                print(&qlex, "Found 'include' without a subsequent filename: '%s' hm?\n",
                      (char*)QUEX_NAME_TOKEN(map_id_to_name)(token_id));
                break;
            }
            print(&qlex, ">> including: ", (const char*)token->get_text().c_str());
            QUEX_TYPE_CHARACTER* tmp = (QUEX_TYPE_CHARACTER*)token->get_text().c_str();
            qlex.include_push<FILE>(tmp);
        }
        else if( token_id == QUEX_TKN_TERMINATION ) {
            if( qlex.include_pop() == false ) 
                continue_lexing_f = false;
            else 
                print(&qlex, "<< return from include\n");
        }

        ++number_of_tokens;

        // (*) check against 'termination'
    } while( continue_lexing_f );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    return 0;
}

void  
space(size_t N)
{ size_t i = 0; for(i=0; i<N; ++i) printf("    "); }

void  
print_token(QUEX_TYPE_ANALYZER* qlex, QUEX_TYPE_TOKEN* token_p, bool TextF /* = false */)
{ 
    space(qlex->include_depth);
    printf("%i: (%i)", (int)token_p->line_number(), (int)token_p->column_number());
    printf("%s", token_p->type_id_name().c_str());
    if( TextF ) printf("\t'%s'", (char*)token_p->text.c_str());
    printf("\n");
}

void 
print(QUEX_TYPE_ANALYZER* qlex, const char* Str1, 
      const char* Str2 /* = 0x0 */, const char* Str3 /* = 0x0*/)
{
    space(qlex->include_depth);
    printf("%s", Str1);
    if( Str2 != 0x0 ) printf("%s", Str2);
    if( Str3 != 0x0 ) printf("%s", Str3);
    printf("\n");
}
