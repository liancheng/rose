#ifndef __ROSE_SCANNER_TYPES_H__
#define __ROSE_SCANNER_TYPES_H__

#include "quex/lexer.h"

#include <glib.h>

typedef quex_lexer          r_lexer;
typedef quex_token          r_token;
typedef QUEX_TYPE_TOKEN_ID  r_token_id;
typedef QUEX_TYPE_CHARACTER r_char;

typedef struct r_scanner {
    r_lexer* lexer;
    r_token* prev_token;
    r_char*  prev_pos;
    GQueue*  token_queue;
}
r_scanner;

#endif  //  __ROSE_SCANNER_TYPES_H__
