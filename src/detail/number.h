#ifndef __ROSE_DETAIL_NUMBER_H__
#define __ROSE_DETAIL_NUMBER_H__

#include "detail/gmp.h"
#include "rose/sexp.h"

struct RFixnum {
    RType* type;
    mpq_t  real;
    mpq_t  imag;
};

struct RFlonum {
    RType* type;
    double real;
    double imag;
};

#define FIXNUM_TO_SEXP(fixnum)  ((rsexp) (fixnum))
#define FIXNUM_FROM_SEXP(obj)   ((RFixnum*) (obj))

#define FLONUM_TO_SEXP(flonum)  ((rsexp) (flonum))
#define FLONUM_FROM_SEXP(obj)   ((RFlonum*) (obj))

#endif  //  __ROSE_DETAIL_NUMBER_H__
