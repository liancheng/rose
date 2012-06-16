#ifndef __ROSE_DETAIL_SEXP_H__
#define __ROSE_DETAIL_SEXP_H__

#include "rose/context.h"
#include "rose/sexp.h"

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

RType* r_sexp_get_type               (rsexp     obj,
                                      RContext* context);
void   r_register_immediate_types    (RContext* context);

#endif  //  __ROSE_DETAIL_SEXP_H__
