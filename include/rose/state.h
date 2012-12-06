#ifndef __ROSE_STATE_H__
#define __ROSE_STATE_H__

#include "rose/sexp.h"
#include "rose/types.h"

typedef struct RState RState;

typedef rpointer (*RAllocFunc) (rpointer, rpointer, rsize);

RState* r_state_open       ();
RState* r_state_new        (RAllocFunc alloc_fn,
                            rpointer   aux);
void    r_state_free       (RState*    state);
rsexp   r_last_error       (RState*    state);
void    r_set_last_error_x (RState*    state,
                            rsexp      error);
void    r_inherit_errno_x  (RState*    state,
                            rint       errnum);

#endif  /* __ROSE_STATE_H__ */
