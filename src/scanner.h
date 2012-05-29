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
void      r_scanner_init          (RScanner* scanner,
                                   rsexp     port);
RToken*   r_scanner_next_token    (RScanner* scanner,
                                   rsexp     port);
RToken*   r_scanner_peek_token    (RScanner* scanner,
                                   rsexp     port);
rtokenid  r_scanner_peek_id       (RScanner* scanner,
                                   rsexp     port);
void      r_scanner_consume_token (RScanner* scanner,
                                   rsexp     port);
RToken*   r_scanner_copy_token    (RToken*   token);
void      r_scanner_free_token    (RToken*   token);
int       r_scanner_line          (RScanner* scanner);
int       r_scanner_column        (RScanner* scanner);

#define RETURN_ON_EOF_OR_FAIL(scanner, port)\
        do {\
            if (TKN_EOF == r_scanner_peek_id (scanner, port)) {\
                return R_SEXP_EOF;\
            }\
            else if (TKN_FAIL == r_scanner_peek_id (scanner, port)) {\
                return R_SEXP_UNSPECIFIED;\
            }\
        }\
        while (0)

#endif  //  __ROSE_DETAIL_SCANNER_H__
