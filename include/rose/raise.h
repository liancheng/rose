#ifndef __ROSE_RAISE_H__
#define __ROSE_RAISE_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef struct RErrorJmp RErrorJmp;

void r_raise (RState* state, rsexp obj);

#endif  /* __ROSE_RAISE_H__ */
