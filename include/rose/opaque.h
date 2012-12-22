#ifndef __ROSE_OPAQUE_H__
#define __ROSE_OPAQUE_H__

#include "rose/sexp.h"

typedef void (*ROpaqueGcMark)     (RState* state, rpointer opaque);
typedef void (*ROpaqueGcFinalize) (RState* state, rpointer opaque);

rsexp    r_opaque_new (RState*           state,
                       rpointer          opaque,
                       ROpaqueGcMark     mark_fn,
                       ROpaqueGcFinalize finalize_fn);
rbool    r_opaque_p   (rsexp             obj);
rpointer r_opaque_get (rsexp             obj);

#endif  //  __ROSE_OPAQUE_H__
