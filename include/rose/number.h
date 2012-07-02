#ifndef __ROSE_NUMBER_H__
#define __ROSE_NUMBER_H__

#include "rose/sexp.h"

#include <gmp.h>

#define r_int_to_sexp(n)        ((rsexp) (((n) << 2) | R_INT30_TAG))
#define r_int_from_sexp(obj)    (((int) (obj)) >> 2)
#define r_int30_p(obj)          R_TC2_EQ_P ((obj), R_INT30_TAG)
#define R_ZERO                  r_int_to_sexp (0)
#define R_ONE                   r_int_to_sexp (1)
#define INT30_MAX               ((1 << 29) - 1)
#define INT30_MIN               (-((1 << 29)))
#define r_fixnum_p(obj)         R_TC3_EQ_P ((obj), R_FIXNUM_TAG)
#define r_flonum_p(obj)         R_TC3_EQ_P ((obj), R_FLONUM_TAG)

typedef struct RFixnum RFixnum;
typedef struct RFlonum RFlonum;

rboolean r_number_p          (rsexp       obj);
rboolean r_byte_p            (rsexp       obj);
rsexp    r_fixnum_new        (mpq_t       real,
                              mpq_t       imag);
rsexp    r_flonum_new        (double      real,
                              double      imag);
void     r_fixnum_set_real_x (rsexp       obj,
                              mpq_t       real);
void     r_fixnum_set_imag_x (rsexp       obj,
                              mpq_t       imag);
void     r_flonum_set_real_x (rsexp       obj,
                              double      real);
void     r_flonum_set_imag_x (rsexp       obj,
                              double      imag);
rsexp    r_string_to_number  (char const* text);

#endif  //  __ROSE_NUMBER_H__
