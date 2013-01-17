#ifndef __ROSE_DETAIL_GC_H__
#define __ROSE_DETAIL_GC_H__

#include "rose/gc.h"
#include "rose/sexp.h"

R_BEGIN_DECLS

typedef enum {
    R_GC_COLOR_WHITE,
    R_GC_COLOR_GRAY,
    R_GC_COLOR_BLACK
}
RGcColor;

typedef struct RGc RGc;

struct RGc {
    rbool     enabled;

    rsexp*    arena;
    rsize     arena_size;
    rsize     arena_index;
    rsize     arena_last_index;

    rsize     n_live;
    rsize     n_live_after_mark;
    rsize     threshold;

    RObject*  gray_list;
    RObject*  chrono_list;
};

#define r_object_new(r, type, tag)\
        (r_cast (type*, r_object_alloc (r, tag)))

RObject* r_object_alloc    (RState* r,
                            RTypeTag type_tag);
void     r_object_free     (RState* r,
                            RObject* obj);

void     gc_enable         (RState* r);
void     gc_disable        (RState* r);
rbool    gc_enabled_p      (RState* r);
void     gc_scope_reset    (RState* r);
void     gc_init           (RState* r);
void     gc_finish         (RState* r);

R_END_DECLS

#endif /* __ROSE_DETAIL_GC_H__ */
