#ifndef __ROSE_DETAIL_STATE_H__
#define __ROSE_DETAIL_STATE_H__

#include "detail/gc.h"
#include "detail/sexp.h"
#include "detail/vm.h"
#include "rose/error.h"
#include "rose/state.h"

#include <glib.h>

R_BEGIN_DECLS

/* Records various execution state of the ROSE interpreter. */
struct RState {
    /* State of the garbage collector */
    RGc gc;

    /* State of the virtual machine */
    RVm vm;

    /* Error handling */
    rsexp last_error;

    /* Memory allocation function */
    RAllocFunc alloc_fn;
    rpointer alloc_aux;

    /* Runtime data */
    rsexp current_input_port;
    rsexp current_output_port;
    rsexp current_error_port;

    /* Global constants */
    rsexp oom_error;
    rsexp flo_zero;
    rsexp flo_one;

    /* Type information for builtin types */
    RTypeInfo builtin_types [R_TAG_MAX];
};

void init_builtin_type (RState* r, RTypeTag tag, RTypeInfo* type);

R_END_DECLS

#endif /* __ROSE_DETAIL_STATE_H__ */
