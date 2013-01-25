#ifndef __ROSE_DETAIL_FINER_NUMBER_H__
#define __ROSE_DETAIL_FINER_NUMBER_H__

#include "detail/sexp.h"
#include "rose/gmp.h"
#include "rose/number.h"

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
#define fixreal_from_sexp(n)    (r_cast (RFixreal*, n))
#define fixreal_value(n)        (fixreal_from_sexp (n)->value)

#define floreal_to_sexp(n)      (r_cast (rsexp, n))
#define floreal_from_sexp(n)    (r_cast (RFloreal*, n))
#define floreal_value(n)        (floreal_from_sexp (n)->value)

#define complex_to_sexp(n)      (r_cast (rsexp, n))
#define complex_from_sexp(n)    (r_cast (RComplex*, n))
#define complex_real(n)         (complex_from_sexp (n)->real)
#define complex_imag(n)         (complex_from_sexp (n)->imag)

rsexp try_small_int     (mpq_t);
rsexp try_small_int_si  (rint num, rint den);
rsexp try_small_int_ui  (ruint num, ruint den);
rsexp fixreal_normalize (rsexp n);

rsexp smi_to_fixreal (RState* r, rsexp n);
rsexp smi_to_floreal (RState* r, rsexp n);

R_END_DECLS

#endif /* __ROSE_DETAIL_FINER_NUMBER_H__ */
