#ifndef __ROSE_ENV_H__
#define __ROSE_ENV_H__

#include "rose/sexp.h"

typedef struct REnv REnv;

rbool r_env_p      (rsexp obj);
rsexp r_env_new    ();
rsexp r_env_extend (rsexp parent,
                    rsexp vars,
                    rsexp vals);
rsexp r_env_lookup (rsexp env,
                    rsexp var);
void  r_env_define (rsexp env,
                    rsexp var,
                    rsexp val);
void  r_env_set_x  (rsexp env,
                    rsexp var,
                    rsexp val);

#endif  //  __ROSE_ENV_H__
