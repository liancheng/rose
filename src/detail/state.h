#ifndef __ROSE_DETAIL_STATE_H__
#define __ROSE_DETAIL_STATE_H__

#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/vm.h"
#include "rose/error.h"
#include "rose/state.h"
#include "rose/symbol.h"

#include <glib.h>

R_BEGIN_DECLS

typedef enum {
    KW_QUOTE,
    KW_LAMBDA,
    KW_IF,
    KW_SET_X,
    KW_BEGIN,
    KW_COND,
    KW_AND,
    KW_OR,
    KW_CASE,
    KW_LET,
    KW_LET_S,
    KW_LETREC,
    KW_DO,
    KW_DELAY,
    KW_QUASIQUOTE,
    KW_ELSE,
    KW_ARROW,
    KW_DEFINE,
    KW_UNQUOTE,
    KW_UNQUOTE_SPLICING,
    KW_CALL_CC,

    INS_APPLY,
    INS_ARG,
    INS_ASSIGN,
    INS_BRANCH,
    INS_CAPTURE_CC,
    INS_CLOSE,
    INS_CONST,
    INS_BIND,
    INS_FRAME,
    INS_HALT,
    INS_REFER,
    INS_RESTORE_CC,
    INS_RETURN,

    RESERVED_WORD_COUNT
}
RReservedWord;

typedef struct RSymbolState {
    GHashTable* index_to_symbol;
    rsize symbol_index;
    rconstcstring* symbols;
}
RSymbolState;

/* Records various execution state of the ROSE interpreter. */
struct RState {
    /* State of the garbage collector */
    RGcState gc;

    /* State of the virtual machine */
    RVm vm;

    /* State for symbol internalization */
    RSymbolState symbol_state;

    /* Error handling */
    rsexp last_error;
    RNestedJump* error_jmp;

    /* Memory allocation function */
    RAllocFunc alloc_fn;
    rpointer alloc_aux;

    /* Runtime data */
    rsexp current_input_port;
    rsexp current_output_port;
    rsexp current_error_port;

    /* Reserved words */
    rsexp reserved [RESERVED_WORD_COUNT];

    /* Type information for builtin types */
    RTypeInfo builtin_types [R_TAG_MAX];
};

void  init_builtin_type (RState* state,
                         RTypeTag tag,
                         RTypeInfo* type);
rsexp reserved          (RState* state,
                         RReservedWord index);

R_END_DECLS

#endif  /* __ROSE_DETAIL_STATE_H__ */
