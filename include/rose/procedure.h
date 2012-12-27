#ifndef __ROSE_PROCEDURE_H__
#define __ROSE_PROCEDURE_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

rsexp r_procedure_new  (RState* state,
                        rsexp body,
                        rsexp env,
                        rsexp vars);
rbool r_procedure_p    (rsexp obj);
rsexp r_procedure_body (rsexp obj);
rsexp r_procedure_env  (rsexp obj);
rsexp r_procedure_vars (rsexp obj);

R_END_DECLS

#endif  //  __ROSE_PROCEDURE_H__
