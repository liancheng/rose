#ifndef __ROSE_MEMORY_H__
#define __ROSE_MEMORY_H__

#include "rose/state.h"

R_BEGIN_DECLS

rpointer r_alloc   (RState* state,
                    rsize size);
rpointer r_realloc (RState* state,
                    rpointer ptr,
                    rsize size);
rpointer r_calloc  (RState* state,
                    rsize element_size,
                    rsize count);
void     r_free    (RState* state,
                    rpointer ptr);

#define r_new(state, type)\
        ((type*) r_alloc (state, sizeof (type)))

#define r_new0(state, type)\
        ((type*) r_calloc (state, sizeof (type), 1u))

#define r_new_array(state, type, n)\
        ((type*) r_alloc (state, sizeof (type) * (n)))

#define r_new0_array(state, type, n)\
        ((type*) r_calloc (state, sizeof (type), (n)))

void r_gc_scope_open    (RState* state);
void r_gc_scope_close   (RState* state);
void r_gc_scope_protect (RState* state,
                         rsexp obj);

void r_full_gc          (RState* state);
void r_gc_mark          (RState* state,
                         rsexp obj);

#define r_gc_scope_close_and_protect(state, obj)\
        do {\
            r_gc_scope_close (state);\
            r_gc_scope_protect (state, obj);\
        }\
        while (0)

R_END_DECLS

#endif  //  __ROSE_MEMORY_H__
