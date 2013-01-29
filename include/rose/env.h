#ifndef __ROSE_ENV_H__
#define __ROSE_ENV_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

rsexp r_env_lookup   (RState* r, rsexp env, rsexp var);
rsexp r_empty_env    (RState* r);
rsexp r_env_bind_x   (RState* r, rsexp env, rsexp var, rsexp val);
rsexp r_env_assign_x (RState* r, rsexp env, rsexp var, rsexp val);

R_END_DECLS

#endif /* __ROSE_ENV_H__ */
