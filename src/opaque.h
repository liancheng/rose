#ifndef __ROSE_DETAIL_OPAQUE_H__
#define __ROSE_DETAIL_OPAQUE_H__

#include "rose/sexp.h"

rsexp    r_opaque_new   (rpointer opaque);
rpointer r_opaque_get   (rsexp    opaque);
void     r_opaque_set_x (rsexp    sexp,
                         rpointer opaque);

#endif  //  __ROSE_DETAIL_OPAQUE_H__
