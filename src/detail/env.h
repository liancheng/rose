#ifndef __ROSE_DETAIL_ENV_H__
#define __ROSE_DETAIL_ENV_H__

#include "rose/sexp.h"
#include "rose/state.h"

rsexp lookup    (RState* state, rsexp env, rsexp var);
rsexp extend    (RState* state, rsexp env, rsexp vars, rsexp vals);
rsexp empty_env (RState* state);
rsexp bind_x    (RState* state, rsexp env, rsexp var, rsexp val);
rsexp assign_x  (RState* state, rsexp env, rsexp var, rsexp val);

#endif  //  __ROSE_DETAIL_ENV_H__
