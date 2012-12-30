#ifndef __ROSE_NATIVE_H__
#define __ROSE_NATIVE_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

typedef rsexp (*RNativeProc) (RState*, rsexp);

typedef struct RNative RNative;

rsexp r_native_new   (RState* state,
                      rconstcstring name,
                      RNativeProc proc,
                      rsize required,
                      rsize optional,
                      rbool has_rest);
rbool r_native_p     (rsexp obj);
rsexp r_native_apply (RState* state,
                      rsexp native,
                      rsexp args);
void r_match_args    (RState* state,
                      rsexp args,
                      rsize required,
                      rsize optional,
                      rbool rest_p,
                      ...);

R_END_DECLS

#endif  //  __ROSE_NATIVE_H__
