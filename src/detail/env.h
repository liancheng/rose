#ifndef __ROSE_DETAIL_ENV_H__
#define __ROSE_DETAIL_ENV_H__

#include "rose/env.h"
#include "rose/primitive.h"

R_BEGIN_DECLS

rsexp env_extend      (RState* state,
                       rsexp env,
                       rsexp vars,
                       rsexp vals);
rsexp default_env     (RState* state);
void bind_primitive_x (RState* state,
                       rsexp* env,
                       rconstcstring name,
                       RPrimitiveFunc func,
                       rsize required,
                       rsize optional,
                       rbool rest_p);

R_END_DECLS

#endif  //  __ROSE_DETAIL_ENV_H__
