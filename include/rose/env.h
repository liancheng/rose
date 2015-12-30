#ifndef __ROSE_ENV_H__
#define __ROSE_ENV_H__

#include "rose/sexp.h"
#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

/**
 * Looks up the value of variable `var` from environment `env`.
 */
rsexp r_env_lookup (RState* r, rsexp env, rsexp var);

/**
 * Creates an empty environment.
 */
rsexp r_empty_env (RState* r);

/**
 * Binds variable `var` to value `val` within environment `env`.
 */
rsexp r_env_bind_x (RState* r, rsexp env, rsexp var, rsexp val);

/**
 * Assigns value `val` to variable `var` within environment `env`.
 *
 * Equivalent to scheme `set!`.
 */
rsexp r_env_assign_x (RState* r, rsexp env, rsexp var, rsexp val);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_ENV_H__ */
