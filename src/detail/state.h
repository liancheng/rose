#ifndef __ROSE_DETAIL_STATE_H__
#define __ROSE_DETAIL_STATE_H__

#include "detail/gc.h"
#include "detail/sexp.h"
#include "rose/error.h"
#include "rose/state.h"
#include "rose/symbol.h"

#include <glib.h>

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
    /* GC */
    RGcState     gc;

    /* Error handling */
    rsexp        last_error;
    RNestedJump* error_jmp;

    /* Memory allocation function */
    RAllocFunc   alloc_fn;
    rpointer     alloc_aux;

    /* Runtime data */
    rsexp        current_input_port;
    rsexp        current_output_port;
    rsexp        current_error_port;
    rsexp        keywords [R_KEYWORD_COUNT];

    /* Type information */
    RTypeInfo    builtin_types [R_TAG_MAX];
    GHashTable*  user_type_ht;
};

void  init_builtin_type (RState*    state,
                         RTypeTag   tag,
                         RTypeInfo* type);
rsexp keyword           (RState*    state,
                         ruint      index);

#endif  /* __ROSE_DETAIL_STATE_H__ */
