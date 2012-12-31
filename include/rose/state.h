#ifndef __ROSE_STATE_H__
#define __ROSE_STATE_H__

#include "rose/sexp.h"
#include "rose/types.h"

R_BEGIN_DECLS

typedef struct RState RState;

typedef rpointer (*RAllocFunc) (rpointer, rpointer, rsize);

RState* r_state_open ();
RState* r_state_new  (RAllocFunc alloc_fn,
                      rpointer aux);
void    r_state_free (RState* r);

R_END_DECLS

#endif  /* __ROSE_STATE_H__ */
