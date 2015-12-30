#ifndef __ROSE_DETAIL_VM_H__
#define __ROSE_DETAIL_VM_H__

#include "rose/sexp.h"
#include "rose/state.h"
#include "rose/vm.h"

#include <glib.h>

/// \cond
R_BEGIN_DECLS
/// \endcond

typedef struct RVm RVm;

/// Holds states of a VM.
struct RVm {
    /// Pointer to procedure argument list
    rsexp args;

    /// Pointer to the current environment
    rsexp env;

    /// Pointer to the next instruction to execute
    rsexp next;

    /// Pointer to the top of the execution stack
    rsexp stack;

    /// Usually points to evaluation result value
    rsexp value;

    /// Whether the VM should halt
    rbool halt_p;
};

/// Initializes the VM
void vm_init (RState* r);

/// Destructs the VM
void vm_finish (RState* r);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_DETAIL_VM_H__ */
