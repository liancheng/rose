#ifndef __ROSE_EQ_H__
#define __ROSE_EQ_H__

#include "rose/sexp.h"

rbool r_eqv_p   (rsexp lhs, rsexp rhs);
rbool r_eq_p    (rsexp lhs, rsexp rhs);
rbool r_equal_p (rsexp lhs, rsexp rhs);

#endif  //  __ROSE_EQ_H__
