#ifndef __ROSE_FINER_NUMBER_H__
#define __ROSE_FINER_NUMBER_H__

#include "rose/gmp.h"
#include "rose/sexp.h"
#include "rose/state.h"

#include <limits.h>

R_BEGIN_DECLS

rsexp r_fixreal_new    (RState* r, mpq_t value);
rsexp r_fixreal_new_si (RState* r, rint num, rint den);
rbool r_fixreal_p      (rsexp obj);

rsexp r_floreal_new    (RState* r, double value);
rbool r_floreal_p      (rsexp obj);

rsexp r_complex_new    (RState* r, rsexp real, rsexp imag);
rbool r_complex_p      (rsexp obj);
rbool r_fixcomplex_p   (rsexp obj);
rbool r_flocomplex_p   (rsexp obj);
rsexp r_real_part      (RState* r, rsexp n);
rsexp r_imag_part      (RState* r, rsexp n);

rbool r_zero_p         (rsexp n);
rint  r_sign           (rsexp n);

R_END_DECLS

#endif /* __ROSE_FINER_NUMBER_H__ */
