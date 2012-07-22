#ifndef __ROSE_DETAIL_STATE_H__
#define __ROSE_DETAIL_STATE_H__

#include "rose/sexp.h"
#include "rose/state.h"
#include "rose/symbol.h"

typedef enum {
    R_QUOTE,
    R_LAMBDA,
    R_IF,
    R_SET_X,
    R_BEGIN,
    R_COND,
    R_AND,
    R_OR,
    R_CASE,
    R_LET,
    R_LET_S,
    R_LETREC,
    R_DO,
    R_DELAY,
    R_QUASIQUOTE,
    R_ELSE,
    R_ARROW,
    R_DEFINE,
    R_UNQUOTE,
    R_UNQUOTE_SPLICING,
    R_KEYWORD_COUNT
}
RKeyword;

struct RState {
    RSymbolTable* symbol_table;
    rsexp         env;
    rsexp         current_input_port;
    rsexp         current_output_port;
    RType**       types;
    rsexp         keywords [R_KEYWORD_COUNT];
};

rsexp r_keyword (RState* state, ruint index);

#endif  //  __ROSE_DETAIL_STATE_H__
