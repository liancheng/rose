#include <stdio.h>

#include "EasyLexer.h"
#include "EasyLexer-token_ids.h"

int 
main(int argc, char** argv) 
{        
    QUEX_TYPE_TOKEN*      token_p;
    QUEX_TYPE_TOKEN_ID    token_id;
    QUEX_TYPE_ANALYZER    qlex;
    int                   number_of_tokens = 0;
    bool                  continue_lexing_f = true;
    QUEX_TYPE_CHARACTER*  tmp = 0x0;
    
    QUEX_NAME(construct_file_name)(&qlex, argc == 1 ? "example.txt" : argv[1], 0x0, false);

    printf(",------------------------------------------------------------------------------------\n");
    printf("| [START]\n");

    /* (*) loop until the 'termination' token arrives */
    token_p = qlex.token;
    do {
        /* (*) get next token from the token stream */
        token_id = QUEX_NAME(receive)(&qlex);

        /* (*) print out token information */
        print_token(&qlex, token_p, true);

        if( token_id == QUEX_TKN_INCLUDE ) { 
            token_id = QUEX_NAME(receive)(&qlex);
            print_token(&qlex, token_p, true);
            if( token_id != QUEX_TKN_IDENTIFIER ) {
                continue_lexing_f = false;
                print(&qlex, "Found 'include' without a subsequent filename: '%s' hm?\n",
                      (char*)QUEX_NAME_TOKEN(map_id_to_name)(token_id), 0x0);
                break;
            }
            print(&qlex, ">> including: ", (const char*)token_p->text, 0x0);
            tmp = ((QUEX_TYPE_CHARACTER*)token_p->text);
            QUEX_NAME(include_push)(&qlex, 0x0, tmp, 0x0, 0x0);
        }
        else if( token_id == QUEX_TKN_TERMINATION ) {
            if( QUEX_NAME(include_pop)(&qlex) == false ) 
                continue_lexing_f = false;
            else 
                print(&qlex, "<< return from include\n", 0x0, 0x0);
        }

        ++number_of_tokens;

        /* (*) check against 'termination' */
    } while( continue_lexing_f );

    printf("| [END] number of token = %i\n", (int)number_of_tokens);
    printf("`------------------------------------------------------------------------------------\n");

    QUEX_NAME(destruct)(&qlex);
    return 0;
}

void  
space(size_t N)
{ size_t i = 0; for(i=0; i<N; ++i) printf("    "); }

void  
print_token(QUEX_TYPE_ANALYZER* qlex, QUEX_TYPE_TOKEN* token_p, bool TextF /* = false */)
{ 
    space(qlex->include_depth);
    printf("%i: (%i)", (int)token_p->_line_n, (int)token_p->_column_n);
    printf("%s", QUEX_NAME_TOKEN(map_id_to_name)(token_p->_id));
    if( TextF ) printf("\t'%s'", (char*)token_p->text);
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
