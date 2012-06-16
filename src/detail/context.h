#ifndef __ROSE_DETAIL_CONTEXT_H__
#define __ROSE_DETAIL_CONTEXT_H__

#include "rose/context.h"
#include "rose/sexp.h"
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

struct RContext {
    RSymbolTable* symbol_table;
    rsexp         env;
    rsexp         current_input_port;
    rsexp         current_output_port;
    RType*        tc3_types [1 << R_TC3_BITS];
    RType*        tc5_types [1 << (R_TC5_BITS - R_TC3_BITS)];
    rsexp         keywords [R_KEYWORD_COUNT];
};

rsexp r_keyword (ruint index, RContext* context);

#endif  //  __ROSE_DETAIL_CONTEXT_H__
