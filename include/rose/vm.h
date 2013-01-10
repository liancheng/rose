#ifndef __ROSE_VM_H__
#define __ROSE_VM_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

rsexp r_eval           (RState* r,
                        rsexp code);
rsexp r_eval_from_port (RState* r,
                        rsexp port);

R_END_DECLS

#endif  //  __ROSE_VM_H__
