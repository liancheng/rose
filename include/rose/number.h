#ifndef __ROSE_NUMBER_H__
#define __ROSE_NUMBER_H__

#include "rose/sexp.h"

#include <limits.h>
#include <gmp.h>

/** Parses a number from string \a text. */
rsexp r_string_to_number (char const* text);

/**
 * \defgroup Flonum Float pointer number
 * \{
 */

/** Creates a flonum, whose real part is \a real, imaginary part is \a imag. */
rsexp r_flonum_new (double real, double imag);

/** Predicates whether \a obj is a flonum. */
rbool r_flonum_p (rsexp obj);

/**
 * \def r_inexact_p(obj)
 * Predicates whether \a obj is an inexact number.
 */
#define r_inexact_p(obj) (r_flonum_p (obj))

/** Sets the real part of flonum \a obj to \a real. */
void  r_flonum_set_real_x (rsexp obj, double real);

/** Sets the imaginary part of flonum \a obj to \a imag. */
void r_flonum_set_imag_x (rsexp obj, double imag);

/** \} */

/**
 * \defgroup Fixnum Fixed pointer number
 * \{
 */

/** Creates a fixnum, whose real part is \a real, imaginary part is \a imag. */
rsexp r_fixnum_new (mpq_t real, mpq_t imag);

/** Creates a fixnum, whose real part is \a real, imaginary part is 0. */
rsexp r_fixreal_new (mpq_t real);

/** Converts fixnum \a obj to small integer if possible. */
rsexp r_fixnum_normalize (rsexp obj);

/** Predicates whether \a obj is a fixnum. */
rbool r_fixnum_p (rsexp obj);

/** Sets the imaginary part of fixnum \a obj to \a imag. */
void r_fixnum_set_real_x (rsexp obj, mpq_t real);

/** Sets the real part of fixnum \a obj to \a real. */
void r_fixnum_set_imag_x (rsexp obj, mpq_t imag);

/** Boxes small integer \a n into an s-expression. */
rsexp r_int_to_sexp (rint n);

/** Unboxes a small integer from s-expression \a obj. */
rint r_int_from_sexp (rsexp obj);

/**
 * \def r_small_int_p(obj)
 * Predicates whether \a obj is a small integer.
 */
#define r_small_int_p(obj)  (((obj) & R_SMI_MASK) == R_SMI_TAG)

/** Predicates whether \a obj represents an unsigned byte. */
rbool r_byte_p (rsexp obj);

/** Predicates whether \a obj is a fixnum, a flonum or a small integer. */
rbool r_number_p (rsexp obj);

/** Predicates whether \a obj is an exact number. */
rbool r_exact_p (rsexp obj);

/**
 * \def R_ZERO
 * Small integer constant 0.
 */
#define R_ZERO r_int_to_sexp (0)

/**
 * \def R_ONE
 * Small integer constant 1.
 */
#define R_ONE r_int_to_sexp (1)

#define R_SEXP_BITS (sizeof (rsexp) * CHAR_BIT)

/**
 * \def R_SMI_MAX
 * Maximum value of signed small integer.
 */
#define R_SMI_MAX ((1 << (R_SEXP_BITS - R_SMI_BITS - 1)) - 1)

/**
 * \def R_SMI_MAX
 * Minimum value of signed small integer.
 */
#define R_SMI_MIN (-R_SMI_MAX - 1)

/** \} */

#endif  /* __ROSE_NUMBER_H__ */
