#ifndef __ROSE_VECTOR_H__
#define __ROSE_VECTOR_H__

#include "rose/sexp.h"

R_BEGIN_DECLS

rsexp r_vector_new     (RState* r,
                        rsize k,
                        rsexp fill);
rsexp r_vector         (RState* r,
                        rsize k,
                        ...);
rbool r_vector_p       (rsexp obj);
rsexp r_vector_ref     (RState* r,
                        rsexp vector,
                        rsize k);
rsexp r_vector_set_x   (RState* r,
                        rsexp vector,
                        rsize k,
                        rsexp obj);
rsexp r_vector_length  (rsexp vector);
rsexp r_list_to_vector (RState* r,
                        rsexp list);
rsexp r_vector_to_list (RState* r,
                        rsexp vector);

R_END_DECLS

#endif  /* __ROSE_VECTOR_H__ */
