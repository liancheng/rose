#include <stdio.h>
#include<assert.h>

#include "tiny_wlexer.h"

void
get_wfile_input(quex_tiny_wlexer* qlex);

int 
main(int argc, char** argv) 
{        
    quex_Token*       token_p = 0x0;
    quex_tiny_wlexer  qlex;
    int               number_of_tokens = 0;
    const size_t      BufferSize = 1024;
    char              buffer[1024];

    get_wfile_input(&qlex);

    printf(",-----------------------------------------------------------------\n");
    printf("| [START]\n");

    do {
        QUEX_NAME(receive)(&qlex, &token_p);
        /* print out token information */
        printf("%s \n", QUEX_NAME_TOKEN(get_string)(token_p, buffer, BufferSize));
 
        ++number_of_tokens;
    } while( token_p->_id != QUEX_TKN_TERMINATION );

    printf("| [END] number of token = %i\n", number_of_tokens);
    printf("`-----------------------------------------------------------------\n");

    QUEX_NAME(destruct)(&qlex);

    return 0;
}

void
get_wfile_input(quex_tiny_wlexer* qlex)
{
    /* We write the file ourselves so that there is never an issue about alignment */
    wchar_t    original[] = L"bonjour le monde hello world hallo welt";
    uint8_t*   End        = (uint8_t*)(original + sizeof(original)/sizeof(wchar_t));
    uint8_t*   p          = 0x0;
    FILE*      fh         = 0x0;

    fh = fopen("wchar_t-example.txt", "w");

    /* Write the wchar_t byte by byte as we have it in memory */
    for(p = (uint8_t*)original; p != End; ++p) fputc(*p, fh);
    fclose(fh);

    /* Normal File Input */
    printf("## FILE* (stdio.h):\n");
    printf("##    Note this works only when engine is generated with -b wchart_t\n");
    printf("##    and therefore QUEX_TYPE_CHARACTER == uint16_t.\n");

    assert(sizeof(QUEX_TYPE_CHARACTER) == sizeof(wchar_t));

    QUEX_NAME(construct_file_name)(qlex, "wchar_t-example.txt", 0x0, false);
}

