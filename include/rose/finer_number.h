#ifndef __ROSE_FINER_NUMBER_H__
#define __ROSE_FINER_NUMBER_H__

#include "rose/gmp.h"
#include "rose/sexp.h"
#include "rose/state.h"

#include <limits.h>

R_BEGIN_DECLS

rsexp r_fixreal_new    (RState* r, mpq_t value);
rsexp r_fixreal_new_si (RState* r, rint num, rint den);
rsexp r_floreal_new    (RState* r, double real);

rbool r_fixreal_p (rsexp obj);
rbool r_floreal_p (rsexp obj);

R_END_DECLS

#endif /* __ROSE_FINER_NUMBER_H__ */
