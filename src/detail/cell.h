#ifndef __ROSE_DETAIL_CELL_H__
#define __ROSE_DETAIL_CELL_H__

#include "detail/hash.h"
#include "rose/sexp.h"

#include <stdio.h>

struct REnv{
    rsexp       parent;
    RHashTable* bindings;
};

struct RError {
    rsexp message;
    rsexp irritants;
};

struct RPair {
    rsexp car;
    rsexp cdr;
};

struct RPort {
    FILE* stream;
    rint  mode;
    rsexp name;
};

struct RString {
    rsize length;
    char* data;
};

struct RVector {
    rsize  length;
    rsexp* data;
};

typedef struct RCell {
    rint type;

    union {
        struct REnv    env;
        struct RString string;
        struct RVector vector;
        struct RPort   port;
        struct RError  error;
    }
    value;
}
RCell;

#define r_cell_p(s)         (((s) & R_SEXP_TAG_MASK) == R_SEXP_CELL_TAG)
#define R_CELL_VALUE(obj)   ((RCell*) obj)->value

RCellType r_cell_get_type   (rsexp     obj);
void      r_cell_set_type_x (rsexp     obj,
                             RCellType type);

#endif  //  __ROSE_DETAIL_CELL_H__
