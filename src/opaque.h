#ifndef __ROSE_DETAIL_OPAQUE_H__
#define __ROSE_DETAIL_OPAQUE_H__

#include "rose/sexp.h"

rsexp    r_opaque_new (rpointer opaque);
rboolean r_opaque_p   (rsexp    obj);
rpointer r_opaque_get (rsexp    opaque);
void     r_opaque_set (rsexp    obj,
                       rpointer opaque);

#endif  //  __ROSE_DETAIL_OPAQUE_H__
