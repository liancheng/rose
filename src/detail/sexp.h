#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/sexp.h"
#include "rose/state.h"

struct RType {
    rsize            cell_size;
    char const*      name;
    RWriteFunction   write_fn;
    RDisplayFunction display_fn;
};

struct RCell {
    RType* type;
};

#define R_CELL_TYPE(obj) (((RCell*) (obj))->type)

RType* r_sexp_get_type      (RState* state,
                             rsexp   obj);
void   r_register_tc3_types (RState* state);

#endif  //  __ROSE_DETAIL_SEXP_H__
