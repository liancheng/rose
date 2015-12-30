#ifndef __ROSE_STATE_H__
#define __ROSE_STATE_H__

#include "rose/sexp.h"
#include "rose/types.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

typedef struct RState RState;

typedef rpointer (*RAllocFunc) (rpointer, rpointer, rsize);

RState* r_state_open ();

RState* r_state_new (RAllocFunc alloc_fn, rpointer aux);

void r_state_free (RState* r);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_STATE_H__ */
