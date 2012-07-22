#ifndef __ROSE_NUMBER_H__
#define __ROSE_NUMBER_H__

#include "rose/sexp.h"

#include <gmp.h>

#define r_int_to_sexp(n)        ((rsexp) (((n) << R_SMI_BITS) | R_SMI_TAG))
#define r_int_from_sexp(obj)    (((rint) (obj)) >> R_SMI_BITS)
#define r_smi_p(obj)            (((obj) & R_SMI_MASK) == R_SMI_TAG)

#define R_ZERO                  r_int_to_sexp (0)
#define R_ONE                   r_int_to_sexp (1)
#define SMI_MAX                 ((1 << (sizeof (rsexp) - 2)) - 1)
#define SMI_MIN                 (-((1 << (sizeof (rsexp) - 2))))

typedef struct RFixnum RFixnum;
typedef struct RFlonum RFlonum;

rsexp r_fixnum_new        (mpq_t       real,
                           mpq_t       imag);
rsexp r_flonum_new        (double      real,
                           double      imag);

rbool r_number_p          (rsexp       obj);
rbool r_byte_p            (rsexp       obj);
rbool r_fixnum_p          (rsexp       obj);
rbool r_flonum_p          (rsexp       obj);

void  r_fixnum_set_real_x (rsexp       obj,
                           mpq_t       real);
void  r_fixnum_set_imag_x (rsexp       obj,
                           mpq_t       imag);
void  r_flonum_set_real_x (rsexp       obj,
                           double      real);
void  r_flonum_set_imag_x (rsexp       obj,
                           double      imag);
rsexp r_string_to_number  (char const* text);

#endif  //  __ROSE_NUMBER_H__
