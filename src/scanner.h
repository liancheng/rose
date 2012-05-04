#ifndef __ROSE_DETAIL_SCANNER_H__
#define __ROSE_DETAIL_SCANNER_H__

#include "quex/lexer.h"
#include "rose/context.h"

typedef quex_lexer          RLexer;
typedef quex_token          RToken;
typedef QUEX_TYPE_TOKEN_ID  rtokenid;
typedef QUEX_TYPE_CHARACTER rchar;

typedef struct RScanner RScanner;

RScanner* r_scanner_new           ();
void      r_scanner_init          (FILE*   input,
                                   rsexp   context);
RToken*   r_scanner_next_token    (FILE*   input,
                                   rsexp   context);
RToken*   r_scanner_peek_token    (FILE*   input,
                                   rsexp   context);
rtokenid  r_scanner_peek_token_id (FILE*   input,
                                   rsexp   context);
void      r_scanner_consume_token (FILE*   input,
                                   rsexp   context);
RToken*   r_scanner_copy_token    (RToken* token);
void      r_scanner_free_token    (RToken* token);

#endif  //  __ROSE_DETAIL_SCANNER_H__
