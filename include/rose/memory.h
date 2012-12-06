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

#define r_new(state, type)\
        ((type*) r_alloc (state, sizeof (type)))

#define r_new0(state, type)\
        ((type*) r_calloc (state, sizeof (type), 1u))

#define r_new_array(state, type, n)\
        ((type*) r_alloc (state, sizeof (type) * (n)))

#define r_new0_array(state, type, n)\
        ((type*) r_calloc (state, sizeof (type), (n)))

#endif  //  __ROSE_MEMORY_H__
