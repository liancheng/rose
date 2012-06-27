#ifndef __ROSE_BYTEVECTOR_H__
#define __ROSE_BYTEVECTOR_H__

#include "rose/sexp.h"

typedef struct RBytevector RBytevector;

rsexp   r_bytevector_new        (rsize      k);
rsexp   r_make_bytevector       (rsize      k,
                                 uint8_t    fill);
rsexp   r_bytevector_set_x      (rsexp      obj,
                                 rsize      k,
                                 uint8_t    byte);
rsexp   r_list_to_bytevector    (rsexp      list);

#endif  //  __ROSE_BYTEVECTOR_H__
