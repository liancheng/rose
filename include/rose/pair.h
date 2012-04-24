#ifndef __ROSE_PAIR_H__
#define __ROSE_PAIR_H__

#include "rose/sexp.h"

#define SEXP_PAIR_P(s) (((s) & 0x03) == SEXP_PAIR_TAG)
#define SEXP_LIST_P(s) (SEXP_TRUE == sexp_list_p(s))

r_sexp sexp_cons      (r_sexp car,
                       r_sexp cdr);
r_sexp sexp_car       (r_sexp sexp);
r_sexp sexp_cdr       (r_sexp sexp);
r_sexp sexp_set_car_x (r_sexp pair,
                       r_sexp sexp);
r_sexp sexp_set_cdr_x (r_sexp pair,
                       r_sexp sexp);
r_sexp sexp_reverse   (r_sexp list);
r_sexp sexp_append_x  (r_sexp list,
                       r_sexp value);
r_sexp sexp_list_p    (r_sexp sexp);
r_sexp sexp_list      (size_t count,
                       ...);
size_t sexp_length    (r_sexp list);

#endif  //  __ROSE_PAIR_H__
