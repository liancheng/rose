#ifndef __ROSE_DETAIL_NUMBER_H__
#define __ROSE_DETAIL_NUMBER_H__

#include "detail/gmp.h"

struct RFixnum {
    mpq_t real;
    mpq_t imag;
};

struct RFlonum {
    double real;
    double imag;
};

#define FIXNUM_TO_SEXP(ptr)     (((rsexp) (ptr)) | R_FIXNUM_TAG)
#define FIXNUM_FROM_SEXP(obj)   ((RFixnum*) ((obj) & (~R_FIXNUM_TAG)))

#define FLONUM_TO_SEXP(ptr)     (((rsexp) (ptr)) | R_FLONUM_TAG)
#define FLONUM_FROM_SEXP(obj)   ((RFlonum*) ((obj) & (~R_FLONUM_TAG)))

#endif  //  __ROSE_DETAIL_NUMBER_H__
