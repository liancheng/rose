#ifndef __ROSE_DETAIL_CELL_H__
#define __ROSE_DETAIL_CELL_H__

#include "rose/env.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/vector.h"

typedef struct RCell {
    rint type;

    union {
        REnv     env;
        RString  string;
        RVector  vector;
        RPort    port;
        RError   error;
        rpointer opaque;
    }
    value;
}
RCell;

#define r_cell_p(s)         (((s) & R_SEXP_TAG_MASK) == R_SEXP_CELL_TAG)
#define R_CELL_VALUE(obj)   ((RCell*) obj)->value

RCellType r_cell_get_type (rsexp     obj);
void      r_cell_set_type (rsexp     obj,
                           RCellType type);

#endif  //  __ROSE_DETAIL_CELL_H__
