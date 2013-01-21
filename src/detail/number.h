#ifndef __ROSE_DETAIL_NUMBER_H__
#define __ROSE_DETAIL_NUMBER_H__

#include "detail/sexp.h"
#include "rose/gmp.h"
#include "rose/number.h"

R_BEGIN_DECLS

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

typedef struct {
    R_OBJECT_HEADER
    rsexp real;
    rsexp imag;
}
RComplex;

typedef struct {
    R_OBJECT_HEADER
    mpq_t value;
}
RFixreal;

typedef struct {
    R_OBJECT_HEADER
    double value;
}
RFloreal;

#define fixreal_to_sexp(n)      (r_cast (rsexp, n))
#define floreal_to_sexp(n)      (r_cast (rsexp, n))

#define fixreal_from_sexp(n)    (r_cast (RFixreal*, n))
#define floreal_from_sexp(n)    (r_cast (RFloreal*, n))

#define fixnum_to_sexp(fixnum)  ((rsexp) (fixnum))
#define fixnum_from_sexp(obj)   ((RFixnum*) (obj))

#define flonum_to_sexp(flonum)  ((rsexp) (flonum))
#define flonum_from_sexp(obj)   ((RFlonum*) (obj))

/** Converts a small integer to a non-normalized fixnum */
rsexp smi_to_fixnum (RState* r, rsexp num);

rsexp smi_to_fixreal (RState* r, rsexp num);

R_END_DECLS

#endif /* __ROSE_DETAIL_NUMBER_H__ */
