#ifndef __ROSE_PAIR_H__
#define __ROSE_PAIR_H__

#include "rose/sexp.h"

#define SEXP_PAIR_P(s) (((s) & 0x03) == SEXP_PAIR_TAG)
#define SEXP_LIST_P(s) (SEXP_TRUE == sexp_list_p(s))

rsexp sexp_cons      (rsexp  car,
                      rsexp  cdr);
rsexp sexp_car       (rsexp  sexp);
rsexp sexp_cdr       (rsexp  sexp);
rsexp sexp_set_car_x (rsexp  pair,
                      rsexp  sexp);
rsexp sexp_set_cdr_x (rsexp  pair,
                      rsexp  sexp);
rsexp sexp_reverse   (rsexp  list);
rsexp sexp_append_x  (rsexp  list,
                      rsexp  value);
rsexp sexp_list_p    (rsexp  sexp);
rsexp sexp_list      (rsize  count,
                      ...);
rsize sexp_length    (rsexp  list);

#endif  //  __ROSE_PAIR_H__
