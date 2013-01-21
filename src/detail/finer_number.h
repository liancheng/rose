#ifndef __ROSE_DETAIL_FINER_NUMBER_H__
#define __ROSE_DETAIL_FINER_NUMBER_H__

#include "detail/sexp.h"
#include "rose/gmp.h"
#include "rose/finer_number.h"

R_BEGIN_DECLS

typedef struct RFixreal RFixreal;

struct RFixreal {
    R_OBJECT_HEADER
    mpq_t value;
};

typedef struct RFloreal RFloreal;

struct RFloreal {
    R_OBJECT_HEADER
    double value;
};

typedef struct RComplex RComplex;

struct RComplex {
    R_OBJECT_HEADER
    rsexp real;
    rsexp imag;
};

#define fixreal_to_sexp(n)      (r_cast (rsexp, n))
#define floreal_to_sexp(n)      (r_cast (rsexp, n))

#define fixreal_from_sexp(n)    (r_cast (RFixreal*, n))
#define floreal_from_sexp(n)    (r_cast (RFloreal*, n))

rsexp smi_to_fixreal (RState* r, rsexp num);

R_END_DECLS

#endif /* __ROSE_DETAIL_FINER_NUMBER_H__ */
