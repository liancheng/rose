#include<fstream>    
#include<iostream> 
#include<sstream> 
#include<cassert>

#include <./tiny_wlexer>

using namespace std;

quex::tiny_wlexer*  get_wstringstream_input();
quex::tiny_wlexer*  get_wfile_input();


int 
main(int argc, char** argv) 
{        
    // (*) create the lexical analyser
    //     if no command line argument is specified user file 'example.txt'
    quex::tiny_wlexer*  qlex = 0x0;

    if( argc < 2 ) {
        cout << "At least one command line argument required.\n";
        return -1;
    } else if ( strcmp(argv[1], "FILE") == 0 ) {
        qlex = get_wfile_input();
    } else if ( strcmp(argv[1], "stringstream") == 0 ) {
        qlex = get_wstringstream_input();
    } else {
        cout << "Experiment " << argv[1] << " not supported by this application.\n";
        return -1;
    }

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";

    int           number_of_tokens = 0;
    // (*) loop until the 'termination' token arrives
    quex::Token*  token_p = qlex->token_p();
    do {
        qlex->receive();
        cout << "(" << qlex->line_number() << ", " << qlex->column_number() << ")  \t";
        cout << string(*token_p);
        cout << endl;
        ++number_of_tokens;
    } while( token_p->type_id() != QUEX_TKN_TERMINATION );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    delete qlex;

    return 0;
}

quex::tiny_wlexer* 
get_wfile_input()
{
    /* We write the file ourselves so that there is never an issue about alignment */
    wchar_t   original[] = L"bonjour le monde\nhello world\nhallo welt\n";
    uint8_t*  End        = (uint8_t*)(original + wcslen(original));
    ofstream   ofile("wchar_t-example.txt");

    /* Write the wchar_t byte by byte as we have it in memory */
    for(uint8_t* p = (uint8_t*)original; p != End; ++p) ofile.put(*p);
    /* Write a terminating zero of the size of wchar_t */
    // for(size_t i=0; i<sizeof(wchar_t); ++i)                ofile.put('\0');
    ofile.close();

    /* Normal File Input */
    cout << "## FILE* (stdio.h):\n";
    cout << "##    Note this works only when engine is generated with -b wchar_t (or no -b)\n";
    cout << "##    and therefore QUEX_TYPE_CHARACTER == wchar_t.\n";
    assert(sizeof(QUEX_TYPE_CHARACTER) == sizeof(wchar_t));
    return new quex::tiny_wlexer("wchar_t-example.txt");
}

quex::tiny_wlexer* 
get_wstringstream_input()
{
    /* Wide String Stream Input */
    std::wstringstream    my_stream;
    cout << "## wstringstream:\n";
    cout << "##    Note this works only when engine is generated with -b wchar_t\n";
    cout << "##    and therefore QUEX_TYPE_CHARACTER == wchar_t.\n";

    assert(sizeof(QUEX_TYPE_CHARACTER) == sizeof(wchar_t));

    my_stream << L"bonjour le monde hello world hallo welt";

    return new quex::tiny_wlexer(&my_stream);
}
