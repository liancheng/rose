#ifndef __ROSE_PROCEDURE_H__
#define __ROSE_PROCEDURE_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

rsexp r_procedure_new (RState* r, rsexp body, rsexp env, rsexp formals);

rbool r_procedure_p (rsexp obj);

rsexp r_procedure_body (rsexp obj);

rsexp r_procedure_env (rsexp obj);

rsexp r_procedure_formals (rsexp obj);

R_END_DECLS

#endif /* __ROSE_PROCEDURE_H__ */
