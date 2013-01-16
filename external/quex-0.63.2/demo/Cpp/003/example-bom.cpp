#include<fstream>    
#include<iostream> 

// (*) include lexical analyser header
#include "EasyLexer"
#include <quex/code_base/bom>
#include <quex/code_base/bom.i>

int 
main(int argc, char** argv) 
{        
    using namespace std;

    quex::Token*          token_p = 0x0;
    // (*) create the lexical analyser
    //     1st arg: input file, default = 'example.txt'
    //     2nd arg: input character encoding name, 0x0 --> no codec conversion
    FILE*                 fh = fopen(argc > 1 ? argv[1] : "example.txt", "rb");

    /* Either there is no BOM, or if there is one, then it must be UTF8 */
    QUEX_TYPE_BOM         bom_type = quex::bom_snap(fh);

    cout << "Found BOM: " << quex::bom_name(bom_type) << endl;

    if( (bom_type & (QUEX_BOM_UTF_8 | QUEX_BOM_NONE)) == 0 ) {
        cout << "Found a non-UTF8 BOM. Exit\n";
        fclose(fh);
        return 0;
    }

    /* The lexer **must** be constructed after the BOM-cut */
    quex::EasyLexer       qlex(fh, "UTF8");


    cout << ",-----------------------------------------------------------------\n";
    cout << "| [START]\n";

    int number_of_tokens = 0;
    do {
        qlex.receive(&token_p);

        cout << "(" << token_p->line_number() << ", " << token_p->column_number() << ")  \t";
        cout << string(*token_p) << endl;
        ++number_of_tokens;

    } while( token_p->type_id() != QUEX_TKN_TERMINATION );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`-----------------------------------------------------------------\n";

    fclose(fh);
    return 0;
}
