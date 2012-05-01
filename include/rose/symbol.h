#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/context.h"
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

RSymbolTable* r_symbol_table_new      ();
rsexp         sexp_from_symbol        (char const* symbol,
                                       RContext*   context);
rsexp         sexp_from_static_symbol (char const* symbol,
                                       RContext*   context);
char const*   sexp_to_symbol          (rsexp       sexp,
                                       RContext*   context);
rsexp         sexp_keywords           ();
rsexp         sexp_keyword            (ruint       name,
                                       RContext*   context);

#endif  //  __ROSE_SYMBOL_H__
