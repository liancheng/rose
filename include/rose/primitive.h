#ifndef __ROSE_PRIMITIVE_H__
#define __ROSE_PRIMITIVE_H__

#include "rose/sexp.h"
#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

typedef rsexp (*RPrimitiveFunc) (RState*, rsexp);

typedef struct RPrimitive RPrimitive;

typedef struct RPrimitiveDesc RPrimitiveDesc;

struct RPrimitiveDesc {
    rconstcstring name;
    RPrimitiveFunc func;
    rsize required;
    rsize optional;
    rbool rest_p;
};

rsexp r_primitive_new (
    RState* r,
    rconstcstring name,
    RPrimitiveFunc func,
    rsize required,
    rsize optional,
    rbool has_rest
);

rbool r_primitive_p (rsexp obj);

rsexp r_primitive_apply (RState* r, rsexp primitive, rsexp args);

void r_match_args (
    RState* r, rsexp args, rsize required, rsize optional, rbool rest_p, ...
);

#define r_check_arg(r, arg, assertion, error_code)\
        if (!(assertion (arg))) {\
            r_error_code ((r), (error_code), (arg));\
            return R_FAILURE;\
        }

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_PRIMITIVE_H__ */
