#ifndef __ROSE_FINER_NUMBER_H__
#define __ROSE_FINER_NUMBER_H__

#include "rose/gmp.h"
#include "rose/sexp.h"
#include "rose/state.h"

#include <limits.h>

R_BEGIN_DECLS

rsexp r_fixint_new     (RState* r, rint real);
rsexp r_fixuint_new    (RState* r, ruint real);

rsexp r_fixreal_new    (RState* r, mpq_t value);
rsexp r_fixreal_new_si (RState* r, rint num, rint den);
rsexp r_fixreal_new_ui (RState* r, ruint num, ruint den);
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
rbool r_byte_p         (rsexp obj);
rbool r_number_p       (rsexp obj);
rbool r_integer_p      (rsexp obj);
rbool r_real_p         (rsexp obj);
rbool r_exact_p        (rsexp obj);
rbool r_inexact_p      (rsexp obj);

rint  r_sign           (rsexp n);

rsexp r_exact_to_inexact (RState* r, rsexp num);
rsexp r_inexact_to_exact (RState* r, rsexp num);

rsexp r_cstr_to_number   (RState* r, rconstcstring text);
rsexp r_string_to_number (RState* r, rsexp text);

rsexp r_int_to_sexp         (rint n);
rint  r_int_from_sexp       (rsexp obj);

#define R_ZERO                  r_int_to_sexp (0)
#define R_ONE                   r_int_to_sexp (1)
#define R_SEXP_BITS             (sizeof (rsexp) * CHAR_BIT)
#define R_SMI_MAX               ((1 << (R_SEXP_BITS - R_SMI_BITS - 1)) - 1)
#define R_SMI_MIN               (-R_SMI_MAX - 1)

#define r_small_int_p(obj)      (((obj) & R_SMI_MASK) == R_TAG_SMI)

#define r_uint_to_sexp(obj)     (r_int_to_sexp (r_cast (rint, (obj))))
#define r_uint_from_sexp(obj)   (r_cast (ruint, r_int_from_sexp ((obj))))

R_END_DECLS

#endif /* __ROSE_FINER_NUMBER_H__ */
