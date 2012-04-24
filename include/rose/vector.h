#ifndef __ROSE_VECTOR_H__
#define __ROSE_VECTOR_H__

#include "rose/sexp.h"

#include <stdlib.h>

#define SEXP_VECTOR_P(s) SEXP_CHECK_TYPE(s, SEXP_VECTOR)

r_sexp sexp_make_vector    (size_t k,
                            r_sexp fill);
r_sexp sexp_vector         (size_t k,
                            ...);
r_sexp sexp_vector_ref     (r_sexp vector,
                            int    k);
r_sexp sexp_vector_set_x   (r_sexp vector,
                            int    k,
                            r_sexp obj);
size_t sexp_vector_length  (r_sexp vector);
r_sexp sexp_list_to_vector (r_sexp list);

#endif  //  __ROSE_VECTOR_H__
