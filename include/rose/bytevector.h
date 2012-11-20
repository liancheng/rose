#ifndef __ROSE_BYTEVECTOR_H__
#define __ROSE_BYTEVECTOR_H__

#include "rose/sexp.h"

typedef struct RBytevector RBytevector;

rsexp r_bytevector_new      (rsize k,
                             rbyte fill);
rsize r_bytevector_length   (rsexp obj);
rbyte r_bytevector_u8_ref   (rsexp obj,
                             rsize k);
rsexp r_bytevector_u8_set_x (rsexp obj,
                             rsize k,
                             rbyte byte);
rsexp r_list_to_bytevector  (rsexp list);
rbool r_bytevector_equal_p  (rsexp lhs,
                             rsexp rhs);

#endif  /* __ROSE_BYTEVECTOR_H__ */
