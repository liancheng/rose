#ifndef __ROSE_NUMBER_H__
#define __ROSE_NUMBER_H__

#include "rose/sexp.h"

#include <gmp.h>

typedef struct RFixnum RFixnum;
typedef struct RFlonum RFlonum;

rsexp   r_fixnum_new        ();
void    r_fixnum_init       (RFixnum*       fixnum);
void    r_fixnum_clear      (RFixnum*       fixnum);
rsexp   r_flonum_new        ();
void    r_fixnum_set_real_x (rsexp          obj,
                             mpq_t          real);
void    r_fixnum_set_imag_x (rsexp          obj,
                             mpq_t          imag);
void    r_flonum_set_real_x (rsexp          obj,
                             double         real);
void    r_flonum_set_imag_x (rsexp          obj,
                             double         imag);
rsexp   r_string_to_number  (char const*    text);

#endif  //  __ROSE_NUMBER_H__
