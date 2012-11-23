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

#define FIXNUM_TO_SEXP(fixnum)  ((rsexp) (fixnum))
#define FIXNUM_FROM_SEXP(obj)   ((RFixnum*) (obj))

#define FLONUM_TO_SEXP(flonum)  ((rsexp) (flonum))
#define FLONUM_FROM_SEXP(obj)   ((RFlonum*) (obj))

rbool r_number_eqv_p (RState* state, rsexp lhs, rsexp rhs);

#endif  /* __ROSE_DETAIL_NUMBER_H__ */
