#ifndef __ROSE_VECTOR_H__
#define __ROSE_VECTOR_H__

#include "rose/sexp.h"

#include <stdlib.h>

#define SEXP_VECTOR_P(s) SEXP_CHECK_TYPE(s, SEXP_VECTOR)

rsexp sexp_make_vector    (rsize k,
                           rsexp fill);
rsexp sexp_vector         (rsize k,
                           ...);
rsexp sexp_vector_ref     (rsexp vector,
                           rsize k);
rsexp sexp_vector_set_x   (rsexp vector,
                           rsize k,
                           rsexp obj);
rsize sexp_vector_length  (rsexp vector);
rsexp sexp_list_to_vector (rsexp list);

#endif  //  __ROSE_VECTOR_H__
