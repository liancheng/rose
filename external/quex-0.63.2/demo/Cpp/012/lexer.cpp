#include<cstdio> 

#include "moritz_Lexer"
#include "max_Lexer"
#include "boeck_Lexer"

/* We do not need to cut the BOM. I include it here only
 * to make sure that it remains 'multi-lexer-safe'.        */
#include <quex/code_base/bom>
#include <quex/code_base/bom.i>

using namespace std;


int 
main(int argc, char** argv) 
{        
    // we want to have error outputs in stdout, so that the unit test could see it.
    max::Lexer     max_lex("example-utf16.txt", "UTF16");
    moritz::Lexer  moritz_lex("example-ucs2.txt", "UCS-2");
    boeck::Lexer   boeck_lex("example-utf8.txt");
    max::Token*    max_token    = 0x0;
    moritz::Token* moritz_token = 0x0;
    boeck::Token*  boeck_token  = 0x0;


    // Each lexer reads one token, since the grammars are similar the lexeme 
    // is always the same.                                                    
    printf("                Max:        Moritz:      Boeck:\n");

    max_token    = max_lex.token_p();
    moritz_token = moritz_lex.token_p();
    boeck_token  = boeck_lex.token_p();
    do {
        (void)max_lex.receive();
        (void)moritz_lex.receive();
        (void)boeck_lex.receive();

        /* Lexeme is same for all three. */
        char* lexeme = (char*)max_token->pretty_char_text().c_str();
        int   L      = (int)max_token->text.length();

        printf("%s", lexeme);

        for(int i=0; i < 10 - L ; ++i) printf(" ");
        printf("\t");
        printf("%s   %s   %s\n", 
               max_token->type_id_name().c_str(), 
               moritz_token->type_id_name().c_str(), 
               boeck_token->type_id_name().c_str());

    } while( boeck_token->type_id() != TKN_TERMINATION );

    return 0;
}

