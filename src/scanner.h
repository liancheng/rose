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
void      r_scanner_init          (rsexp   input,
                                   rsexp   context);
RToken*   r_scanner_next_token    (rsexp   input,
                                   rsexp   context);
RToken*   r_scanner_peek_token    (rsexp   input,
                                   rsexp   context);
rtokenid  r_scanner_peek_token_id (rsexp   input,
                                   rsexp   context);
void      r_scanner_consume_token (rsexp   input,
                                   rsexp   context);
RToken*   r_scanner_copy_token    (RToken* token);
void      r_scanner_free_token    (RToken* token);

#define RETURN_ON_EOF_OR_FAIL(input, context)\
        do {\
            if (TKN_TERMINATION == r_scanner_peek_token_id(input, context)) {\
                return R_SEXP_EOF;\
            }\
            else if (TKN_FAIL == r_scanner_peek_token_id(input, context)) {\
                return R_SEXP_UNSPECIFIED;\
            }\
        }\
        while (0)

#endif  //  __ROSE_DETAIL_SCANNER_H__
