#ifndef __ROSE_SCANNER_H__
#define __ROSE_SCANNER_H__

#include "quex/lexer.h"
#include "rose/context.h"

typedef quex_lexer         r_lexer;
typedef quex_token         r_token;
typedef QUEX_TYPE_TOKEN_ID r_token_id;

r_token*   scanner_next_token    (FILE*      input,
                                  r_context* context);
r_token*   scanner_peek_token    (FILE*      input,
                                  r_context* context);
r_token_id scanner_peek_token_id (FILE*      input,
                                  r_context* context);

#endif  //  __ROSE_SCANNER_H__
