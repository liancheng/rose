#ifndef __ROSE_DETAIL_ERROR_H__
#define __ROSE_DETAIL_ERROR_H__

#include "rose/error.h"

rsexp wrong_type_arg (RState* state,
                      rconstcstring expected,
                      rsexp given);
rsexp bad_syntax     (RState* state,
                      rsexp expr);
rsexp bad_variable   (RState* state,
                      rsexp var,
                      rsexp expr);
rsexp bad_formals    (RState* state,
                      rsexp formals,
                      rsexp expr);

#endif  //  __ROSE_DETAIL_ERROR_H__
