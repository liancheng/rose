#ifndef __ROSE_DETAIL_ENV_H__
#define __ROSE_DETAIL_ENV_H__

#include "rose/env.h"
#include "rose/primitive.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

rsexp env_extend (RState* r, rsexp env, rsexp vars, rsexp vals);

rsexp default_env (RState* r);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_DETAIL_ENV_H__ */
