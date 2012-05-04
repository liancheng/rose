#ifndef __ROSE_PAIR_H__
#define __ROSE_PAIR_H__

#include "rose/sexp.h"

rsexp    r_cons      (rsexp car,
                      rsexp cdr);
rsexp    r_car       (rsexp sexp);
rsexp    r_cdr       (rsexp sexp);
rsexp    r_set_car_x (rsexp pair,
                      rsexp sexp);
rsexp    r_set_cdr_x (rsexp pair,
                      rsexp sexp);
rsexp    r_reverse   (rsexp list);
rsexp    r_append_x  (rsexp list,
                      rsexp value);
rboolean r_list_p    (rsexp sexp);
rsexp    r_list      (rsize count,
                      ...);
rsize    r_length    (rsexp list);

#endif  //  __ROSE_PAIR_H__
