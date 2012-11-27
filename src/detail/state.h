#ifndef __ROSE_DETAIL_STATE_H__
#define __ROSE_DETAIL_STATE_H__

#include "detail/raise.h"
#include "detail/sexp.h"
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
    /* Memory allocation function */
    RAllocFunc    alloc_fn;
    rpointer      alloc_aux;

    /* Runtime data */
    RSymbolTable* symbol_table;
    rsexp         env;
    rsexp         current_input_port;
    rsexp         current_output_port;
    rsexp         current_error_port;
    rsexp         keywords [R_KEYWORD_COUNT];

    /* Error handling */
    rsexp         error;
    RNestedJump*  error_jmp;

    /* Type information */
    RTypeInfo*    types [R_TAG_MAX];
};

rsexp    r_keyword (RState*       state,
                    ruint         index);
rcstring r_strdup  (RState*       state,
                    rconstcstring str);

#endif  /* __ROSE_DETAIL_STATE_H__ */
