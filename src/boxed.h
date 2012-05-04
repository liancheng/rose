#ifndef __ROSE_DETAIL_BOXED_H__
#define __ROSE_DETAIL_BOXED_H__

#include "rose/env.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/vector.h"

typedef struct RBoxed {
    rint type;

    union {
        REnv     env;
        RString  string;
        RVector  vector;
        RPort    port;
        RError   error;
        rpointer opaque;
    }
    value;
}
RBoxed;

#define R_BOXED_P(s)        (((s) & 0x03) == R_SEXP_BOXED_TAG)
#define R_BOXED_VALUE(sexp) ((RBoxed*)sexp)->value

rint     r_boxed_get_type   (rsexp    sexp);
void     r_boxed_set_type   (rsexp    sexp,
                             rint     type);
#endif  //  __ROSE_DETAIL_BOXED_H__
