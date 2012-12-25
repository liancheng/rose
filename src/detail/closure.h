#ifndef __ROSE_DETAIL_CLOSURE_H__
#define __ROSE_DETAIL_CLOSURE_H__

#include "rose/sexp.h"
#include "rose/state.h"

rsexp r_closure_new  (RState* state, rsexp body, rsexp env, rsexp vars);
rbool r_closure_p    (rsexp obj);
rsexp r_closure_body (rsexp obj);
rsexp r_closure_env  (rsexp obj);
rsexp r_closure_vars (rsexp obj);

#endif  //  __ROSE_DETAIL_CLOSURE_H__
