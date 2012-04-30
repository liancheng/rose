#ifndef __ROSE_SCANNER_TYPES_H__
#define __ROSE_SCANNER_TYPES_H__

#include "quex/lexer.h"

typedef quex_lexer          RLexer;
typedef quex_token          RToken;
typedef QUEX_TYPE_TOKEN_ID  rtokenid;
typedef QUEX_TYPE_CHARACTER rchar;

typedef struct RScanner {
    RLexer* lexer;
    RToken* lookahead_token;
}
RScanner;

#endif  //  __ROSE_SCANNER_TYPES_H__
