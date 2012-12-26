#ifndef __ROSE_DETAIL_CLOSURE_H__
#define __ROSE_DETAIL_CLOSURE_H__

#include "rose/sexp.h"
#include "rose/state.h"

rsexp r_procedure_new  (RState* state,
                        rsexp body,
                        rsexp env,
                        rsexp vars);
rbool r_procedure_p    (rsexp obj);
rsexp r_procedure_body (rsexp obj);
rsexp r_procedure_env  (rsexp obj);
rsexp r_procedure_vars (rsexp obj);

#endif  //  __ROSE_DETAIL_CLOSURE_H__
