#ifndef __ROSE_DETAIL_KEYWORD_H__
#define __ROSE_DETAIL_KEYWORD_H__

#include "rose/sexp.h"

enum {
    AND,        ARROW,
    BEGIN,      CASE,
    COND,       DEFINE,
    DELAY,      DO,
    ELSE,       IF,
    LAMBDA,     LET,
    LET_A,      LETREC,
    OR,         QUASIQUOTE,
    QUOTE,      SET_X,
    UNQUOTE,    UNQUOTE_SPLICING,

    N_KEYWORD,
};

rsexp r_keywords_init ();
rsexp r_keyword       (ruint name,
                       rsexp context);

#endif  //  __ROSE_DETAIL_KEYWORD_H__
