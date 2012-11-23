#ifndef __ROSE_MEMORY_H__
#define __ROSE_MEMORY_H__

#include "rose/state.h"

rpointer r_alloc   (RState*  state,
                    rsize    size);
rpointer r_realloc (RState*  state,
                    rpointer ptr,
                    rsize    size);
rpointer r_calloc  (RState*  state,
                    rsize    element_size,
                    rsize    count);
void     r_free    (RState*  state,
                    rpointer ptr);

#define R_NEW(state, type)\
        ((type*) r_alloc (state, sizeof (type)))

#define R_NEW0(state, type)\
        ((type*) r_calloc (state, sizeof (type), 1u))

#endif  //  __ROSE_MEMORY_H__
