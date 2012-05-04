#ifndef __ROSE_ENV_H__
#define __ROSE_ENV_H__

#include "rose/sexp.h"

rsexp r_env_new    ();
rsexp r_env_extend (rsexp parent,
                    rsexp vars,
                    rsexp vals);
rsexp r_env_lookup (rsexp env,
                    rsexp var);
void  r_env_define (rsexp env,
                    rsexp var,
                    rsexp val);
void  r_env_set    (rsexp env,
                    rsexp var,
                    rsexp val);

#endif  //  __ROSE_ENV_H__
