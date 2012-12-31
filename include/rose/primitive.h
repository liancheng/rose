#ifndef __ROSE_PRIMITIVE_H__
#define __ROSE_PRIMITIVE_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

typedef rsexp (*RPrimitiveFunc) (RState*, rsexp);

typedef struct RPrimitive RPrimitive;

rsexp r_primitive_new   (RState* r,
                         rconstcstring name,
                         RPrimitiveFunc func,
                         rsize required,
                         rsize optional,
                         rbool has_rest);
rbool r_primitive_p     (rsexp obj);
rsexp r_primitive_apply (RState* r,
                         rsexp primitive,
                         rsexp args);
void r_match_args       (RState* r,
                         rsexp args,
                         rsize required,
                         rsize optional,
                         rbool rest_p,
                         ...);

R_END_DECLS

#endif  //  __ROSE_PRIMITIVE_H__
