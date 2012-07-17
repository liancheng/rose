#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef rboolean (*REqvPredicate)   (rsexp, rsexp);
typedef rboolean (*REqPredicate)    (rsexp, rsexp);
typedef rboolean (*REqualPredicate) (rsexp, rsexp);

struct RType {
    rsize            cell_size;
    char const*      name;
    RWriteFunction   write;
    RDisplayFunction display;
    REqvPredicate    eqv_p;
    REqPredicate     eq_p;
    REqualPredicate  equal_p;
};

struct RCell {
    RType* type;
};

#define R_CELL_TYPE(obj) (((RCell*) (obj))->type)

RType* r_sexp_get_type      (RState* state,
                             rsexp   obj);
void   r_register_tc3_types (RState* state);

#endif  //  __ROSE_DETAIL_SEXP_H__
