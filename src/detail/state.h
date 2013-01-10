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

/* Records various execution state of the ROSE interpreter. */
struct RState {
    /* State of the garbage collector */
    RGcState gc;

    /* State of the virtual machine */
    RVm vm;

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

    /* Interned keyword symbols */
    struct {
        rsexp quote;
        rsexp lambda;
        rsexp if_;
        rsexp set_x;
        rsexp quasiquote;
        rsexp define;
        rsexp unquote;
        rsexp unquote_splicing;
        rsexp call_cc;
    }
    kw;

    /* Interned instruction name symbols */
    struct {
        rsexp apply;
        rsexp arg;
        rsexp assign;
        rsexp branch;
        rsexp capture_cc;
        rsexp close;
        rsexp constant;
        rsexp bind;
        rsexp frame;
        rsexp halt;
        rsexp refer;
        rsexp restore_cc;
        rsexp return_;
    }
    i;

    /* Type information for builtin types */
    RTypeInfo builtin_types [R_TAG_MAX];
};

void  init_builtin_type (RState* r,
                         RTypeTag tag,
                         RTypeInfo* type);

R_END_DECLS

#endif  /* __ROSE_DETAIL_STATE_H__ */
