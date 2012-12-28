#ifndef __ROSE_DETAIL_ERROR_H__
#define __ROSE_DETAIL_ERROR_H__

#include "rose/error.h"

R_BEGIN_DECLS

rsexp wrong_type_arg        (RState* state,
                             rconstcstring expected,
                             rsexp given);
rsexp bad_syntax            (RState* state,
                             rsexp expr);
rsexp bad_variable          (RState* state,
                             rsexp var,
                             rsexp expr);
rsexp bad_formals           (RState* state,
                             rsexp formals,
                             rsexp expr);
rsexp malformed_instruction (RState* state,
                             rsexp ins);

R_END_DECLS

#endif  //  __ROSE_DETAIL_ERROR_H__
