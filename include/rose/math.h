#ifndef __ROSE_MATH_H__
#define __ROSE_MATH_H__

#include "rose/sexp.h"
#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

rsexp r_add (RState* r, rsexp lhs, rsexp rhs);

rsexp r_negate (RState* r, rsexp n);

rsexp r_minus (RState* r, rsexp lhs, rsexp rhs);

rsexp r_multiply (RState* r, rsexp lhs, rsexp rhs);

rsexp r_invert (RState* r, rsexp n);

rsexp r_divide (RState* r, rsexp lhs, rsexp rhs);

rsexp r_modulo (RState* r, rsexp lhs, rsexp rhs);

rsexp r_num_eq_p (RState* r, rsexp lhs, rsexp rhs);

rsexp r_num_lt_p (RState* r, rsexp lhs, rsexp rhs);

rsexp r_num_le_p (RState* r, rsexp lhs, rsexp rhs);

rsexp r_num_gt_p (RState* r, rsexp lhs, rsexp rhs);

rsexp r_num_ge_p (RState* r, rsexp lhs, rsexp rhs);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_MATH_H__ */
