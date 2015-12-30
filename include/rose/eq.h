#ifndef __ROSE_EQ_H__
#define __ROSE_EQ_H__

#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

/**
 * Equivalent to scheme `eq?`.
 */
rbool r_eq_p (RState* r, rsexp lhs, rsexp rhs);

/**
 * Equivalent to scheme `eqv?`.
 */
rbool r_eqv_p (RState* r, rsexp lhs, rsexp rhs);

/**
 * Equivalent to scheme `equal?`.
 */
rbool r_equal_p (RState* r, rsexp lhs, rsexp rhs);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_EQ_H__ */
