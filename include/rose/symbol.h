#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "context_access.h"
#include "rose/sexp.h"

#define SEXP_SYMBOL_P(s)    (((s) & 0x07) == SEXP_SYMBOL_TAG)

typedef struct RSymbolTable RSymbolTable;

enum {
    AND,            ARROW,
    BEGIN,          CASE,
    COND,           DEFINE,
    DELAY,          DO,
    ELSE,           IF,
    LAMBDA,         LET,
    LET_A,          LETREC,
    OR,             QUASIQUOTE,
    QUOTE,          SET_X,
    UNQUOTE,        UNQUOTE_SPLICING,

    KEYWORD_COUNT,
};

RSymbolTable* r_symbol_table_new ();
rsexp         r_symbol_new       (char const* symbol,
                                  RContext*   context);
rsexp         r_static_symbol    (char const* symbol,
                                  RContext*   context);
char const*   r_symbol_name      (rsexp       sexp,
                                  RContext*   context);
rsexp         r_keywords_init    ();
rsexp         r_keyword          (ruint       name,
                                  RContext*   context);

#endif  //  __ROSE_SYMBOL_H__
