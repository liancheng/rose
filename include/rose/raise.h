#ifndef __ROSE_RAISE_H__
#define __ROSE_RAISE_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

typedef struct RErrorJmp RErrorJmp;

void r_raise (RState* state, rsexp obj);

R_END_DECLS

#endif  /* __ROSE_RAISE_H__ */
