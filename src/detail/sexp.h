#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef rbool (*REqvPredicate)   (rsexp, rsexp);
typedef rbool (*REqPredicate)    (rsexp, rsexp);
typedef rbool (*REqualPredicate) (rsexp, rsexp);

struct RType {
    rsize            size;
    char const*      name;
    RWriteFunction   write;
    RDisplayFunction display;
    REqvPredicate    eqv_p;
    REqPredicate     eq_p;
    REqualPredicate  equal_p;
};

#define R_SEXP_TYPE(obj)    (*(RType**) (obj))

RType* r_sexp_get_type  (RState* state,
                         rsexp   obj);
void   r_register_types (RState* state);

#endif  //  __ROSE_DETAIL_SEXP_H__
