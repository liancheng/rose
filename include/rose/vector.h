#ifndef __ROSE_VECTOR_H__
#define __ROSE_VECTOR_H__

#include "rose/sexp.h"

#include <stdlib.h>

rsexp r_vector_new     (rsize k);
rsexp r_make_vector    (rsize k,
                        rsexp fill);
rsexp r_vector         (rsize k,
                        ...);
rsexp r_vector_ref     (rsexp vector,
                        rsize k);
rsexp r_vector_set_x   (rsexp vector,
                        rsize k,
                        rsexp obj);
rsize r_vector_length  (rsexp vector);
rsexp r_list_to_vector (rsexp list);

#endif  //  __ROSE_VECTOR_H__
