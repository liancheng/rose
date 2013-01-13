#ifndef __ROSE_MATH_H__
#define __ROSE_MATH_H__

#include "rose/sexp.h"

R_BEGIN_DECLS

rsexp r_add      (RState* r, rsexp lhs, rsexp rhs);
rsexp r_minus    (RState* r, rsexp lhs, rsexp rhs);
rsexp r_multiply (RState* r, rsexp lhs, rsexp rhs);
rsexp r_divide   (RState* r, rsexp lhs, rsexp rhs);
rsexp r_modulo   (RState* r, rsexp lhs, rsexp rhs);
rsexp r_equal    (RState* r, rsexp lhs, rsexp rhs);

R_END_DECLS

#endif // __ROSE_MATH_H__
