#ifndef __ROSE_DETAIL_NUMBER_H__
#define __ROSE_DETAIL_NUMBER_H__

#include "detail/gmp.h"
#include "detail/sexp.h"
#include "rose/number.h"

typedef struct {
    R_OBJECT_HEADER
    mpq_t real;
    mpq_t imag;
}
RFixnum;

typedef struct {
    R_OBJECT_HEADER
    double real;
    double imag;
}
RFlonum;

#define fixnum_to_sexp(fixnum)  ((rsexp) (fixnum))
#define fixnum_from_sexp(obj)   ((RFixnum*) (obj))

#define flonum_to_sexp(flonum)  ((rsexp) (flonum))
#define flonum_from_sexp(obj)   ((RFlonum*) (obj))

void init_fixnum_type_info (RState* state);
void init_flonum_type_info (RState* state);

#endif  /* __ROSE_DETAIL_NUMBER_H__ */
