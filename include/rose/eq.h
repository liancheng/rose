#ifndef __ROSE_EQ_H__
#define __ROSE_EQ_H__

#include "rose/state.h"

R_BEGIN_DECLS

rbool r_eqv_p (RState* r, rsexp lhs, rsexp rhs);

rbool r_eq_p (RState* r, rsexp lhs, rsexp rhs);

rbool r_equal_p (RState* r, rsexp lhs, rsexp rhs);

R_END_DECLS

#endif /* __ROSE_EQ_H__ */
