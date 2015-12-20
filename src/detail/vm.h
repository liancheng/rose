#ifndef __ROSE_DETAIL_VM_H__
#define __ROSE_DETAIL_VM_H__

#include "rose/sexp.h"
#include "rose/state.h"
#include "rose/vm.h"

#include <glib.h>

R_BEGIN_DECLS

typedef struct RVm RVm;

struct RVm {
    rsexp args;
    rsexp env;
    rsexp next;
    rsexp stack;
    rsexp value;
    rbool halt_p;
};

void vm_init (RState* r);

void vm_finish (RState* r);

R_END_DECLS

#endif /* __ROSE_DETAIL_VM_H__ */
