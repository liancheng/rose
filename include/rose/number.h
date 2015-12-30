#ifndef __ROSE_FINER_NUMBER_H__
#define __ROSE_FINER_NUMBER_H__

#include "rose/gmp.h"
#include "rose/sexp.h"
#include "rose/state.h"

#include <limits.h>

/// \cond
R_BEGIN_DECLS
/// \endcond

rsexp r_fixreal_new (RState* r, mpq_t value);
rsexp r_fixreal_new_si (RState* r, rintw num, rintw den);
rsexp r_fixreal_new_ui (RState* r, ruintw num, ruintw den);

rsexp r_floreal_new (RState* r, double value);

rsexp r_numerator (RState* r, rsexp n);
rsexp r_denominator (RState* r, rsexp n);

rsexp r_complex_new (RState* r, rsexp real, rsexp imag);
rsexp r_real_part (RState* r, rsexp n);
rsexp r_imag_part (RState* r, rsexp n);

rbool r_zero_p (rsexp n);
rbool r_one_p (rsexp n);

rbool r_byte_p (rsexp obj);
rbool r_integer_p (rsexp obj);
rbool r_real_p (rsexp obj);
rbool r_exact_p (rsexp obj);
rbool r_inexact_p (rsexp obj);

rintw r_sign (rsexp n);
rbool r_positive_p (rsexp n);
rbool r_negative_p (rsexp n);

rsexp r_exact_to_inexact (RState* r, rsexp num);
rsexp r_inexact_to_exact (RState* r, rsexp num);

rsexp r_cstr_to_number (RState* r, rconstcstring text);
rsexp r_string_to_number (RState* r, rsexp text);

rsexp r_int_to_sexp (rintw n);
rintw r_int_from_sexp (rsexp obj);

#define R_ZERO                  r_int_to_sexp (0)
#define R_ONE                   r_int_to_sexp (1)
#define R_SEXP_BITS             (sizeof (rsexp) * CHAR_BIT)
#define R_SMI_MAX               ((1L << (R_SEXP_BITS - R_SMI_BITS - 1)) - 1)
#define R_SMI_MIN               (-R_SMI_MAX - 1)

#define r_small_int_p(obj)      (((obj) & R_SMI_MASK) == R_TAG_SMI)

#define r_fixreal_p(obj)        (r_type_tag (obj) == R_TAG_FIXREAL)
#define r_floreal_p(obj)        (r_type_tag (obj) == R_TAG_FLOREAL)

#define r_fixcomplex_p(obj)     (r_type_tag (obj) == R_TAG_FIX_COMPLEX)
#define r_flocomplex_p(obj)     (r_type_tag (obj) == R_TAG_FLO_COMPLEX)
#define r_complex_p(obj)        (r_fixcomplex_p (obj) || r_flocomplex_p (obj))

#define r_number_p(obj)         (r_exact_p (obj) || r_inexact_p (obj))

#define r_uint_to_sexp(obj)     (r_int_to_sexp (r_cast (rintw, (obj))))
#define r_uint_from_sexp(obj)   (r_cast (ruintw, r_int_from_sexp ((obj))))

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_FINER_NUMBER_H__ */
