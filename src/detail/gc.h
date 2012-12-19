#ifndef __ROSE_DETAIL_GC_H__
#define __ROSE_DETAIL_GC_H__

#include "detail/sexp.h"
#include "rose/gc.h"

typedef struct RGcState RGcState;

struct RGcState {
    RObject** arena;
    rsize     arena_size;
    rsize     arena_index;
    rsize     arena_last_index;
    RObject*  gray_list;
    RObject*  chrono_list;
};

#define r_object_new(state, type, tag)\
        (r_cast (type*, r_object_alloc (state, tag)))

RObject* r_object_alloc    (RState*  state,
                            RTypeTag type_tag);
void     r_object_free     (RState*  state,
                            RObject* obj);

void     gc_scope_reset    (RState* state);
void     gc_state_init     (RState*   state,
                            RGcState* gc);
void     gc_state_destruct (RState*   state,
                            RGcState* gc);

#endif  //  __ROSE_DETAIL_GC_H__
