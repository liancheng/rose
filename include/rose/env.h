#ifndef __ROSE_ENV_H__
#define __ROSE_ENV_H__

#include "rose/sexp.h"

rsexp sexp_env_new    ();
rsexp sexp_env_extend (rsexp parent,
                       rsexp vars,
                       rsexp vals);
rsexp sexp_env_lookup (rsexp env,
                       rsexp var);
void  sexp_env_define (rsexp env,
                       rsexp var,
                       rsexp val);
void  sexp_env_set    (rsexp env,
                       rsexp var,
                       rsexp val);

#endif  //  __ROSE_ENV_H__
