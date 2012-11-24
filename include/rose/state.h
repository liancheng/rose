#ifndef __ROSE_STATE_H__
#define __ROSE_STATE_H__

#include "rose/types.h"

typedef struct RState RState;

typedef rpointer (*RAllocFunc) (RState*, rpointer, rsize, rpointer);

RState*  r_state_open ();
RState*  r_state_new  (RAllocFunc alloc_fn,
                       rpointer   aux);
void     r_state_free (RState*    state);

#endif  /* __ROSE_STATE_H__ */
