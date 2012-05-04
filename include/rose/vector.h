#ifndef __ROSE_VECTOR_H__
#define __ROSE_VECTOR_H__

#include "rose/context.h"
#include "rose/sexp.h"
#include "rose/write.h"

#include <stdlib.h>

typedef struct RVector {
    rsize  size;
    rsexp* data;
}
RVector;

rboolean r_vector_p       (rsexp sexp);
rsexp    r_vector_new     (rsize k);
rsexp    r_make_vector    (rsize k,
                           rsexp fill);
rsexp    r_vector         (rsize k,
                           ...);
rboolean r_vector_equal_p (rsexp lhs,
                           rsexp rhs);
rsexp    r_vector_ref     (rsexp vector,
                           rsize k);
rsexp    r_vector_set_x   (rsexp vector,
                           rsize k,
                           rsexp obj);
rsize    r_vector_length  (rsexp vector);
rsexp    r_list_to_vector (rsexp list);
void     r_write_vector   (FILE* output,
                           rsexp sexp,
                           rsexp context);

#endif  //  __ROSE_VECTOR_H__
