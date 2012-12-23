#ifndef __ROSE_BYTEVECTOR_H__
#define __ROSE_BYTEVECTOR_H__

#include "rose/sexp.h"

R_BEGIN_DECLS

rsexp r_bytevector_new      (RState* state,
                             rsize   k,
                             rbyte   fill);
rbool r_bytevector_p        (rsexp   obj);
rsexp r_bytevector_length   (rsexp   obj);
rsexp r_bytevector_u8_ref   (RState* state,
                             rsexp   obj,
                             rsize   k);
rsexp r_bytevector_u8_set_x (RState* state,
                             rsexp   obj,
                             rsize   k,
                             rbyte   byte);
rsexp r_list_to_bytevector  (RState* state,
                             rsexp   list);

R_END_DECLS

#endif  /* __ROSE_BYTEVECTOR_H__ */
