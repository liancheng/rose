#ifndef __ROSE_PAIR_H__
#define __ROSE_PAIR_H__

#include "rose/context.h"
#include "rose/sexp.h"

#include <stdio.h>

#define r_pair_p(s) (((s) & R_SEXP_TAG_MASK) == R_SEXP_PAIR_TAG)

typedef struct RPair {
    rsexp car;
    rsexp cdr;
}
RPair;

rsexp    r_cons         (rsexp car,
                         rsexp cdr);
rsexp    r_car          (rsexp sexp);
rsexp    r_cdr          (rsexp sexp);
rsexp    r_set_car_x    (rsexp pair,
                         rsexp sexp);
rsexp    r_set_cdr_x    (rsexp pair,
                         rsexp sexp);
rboolean r_pair_equal_p (rsexp lhs,
                         rsexp rhs);
rsexp    r_reverse      (rsexp list);
rsexp    r_append_x     (rsexp list,
                         rsexp value);
rboolean r_list_p       (rsexp sexp);
rsexp    r_list         (rsize count,
                         ...);
rsize    r_length       (rsexp list);
void     r_write_pair   (FILE* output,
                         rsexp sexp,
                         rsexp context);
void     r_write_null   (FILE* output,
                         rsexp sexp,
                         rsexp context);

#endif  //  __ROSE_PAIR_H__
