#ifndef __ROSE_EQ_H__
#define __ROSE_EQ_H__

#include "rose/sexp.h"
#include "rose/types.h"

rboolean r_eqv_p   (rsexp lhs, rsexp rhs);
rboolean r_eq_p    (rsexp lhs, rsexp rhs);
rboolean r_equal_p (rsexp lhs, rsexp rhs);

#endif  //  __ROSE_EQ_H__
