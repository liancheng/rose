#ifndef __ROSE_VECTOR_H__
#define __ROSE_VECTOR_H__

#include "rose/sexp.h"

R_BEGIN_DECLS

typedef struct RVector RVector;

rsexp r_vector_new     (RState* state,
                        rsize   k,
                        rsexp   fill);
rsexp r_vector         (RState* state,
                        rsize   k,
                        ...);
rbool r_vector_p       (rsexp   obj);
rsexp r_vector_ref     (rsexp   vector,
                        rsize   k);
rsexp r_vector_set_x   (rsexp   vector,
                        rsize   k,
                        rsexp   obj);
rsize r_vector_length  (rsexp   vector);
rsexp r_list_to_vector (RState* state,
                        rsexp   list);
rsexp r_vector_to_list (RState* state,
                        rsexp   vector);

R_END_DECLS

#endif  /* __ROSE_VECTOR_H__ */
