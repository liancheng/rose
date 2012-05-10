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

#define r_boxed_p(s)        (((s) & 0x03) == R_SEXP_BOXED_TAG)
#define R_BOXED_VALUE(obj)  ((RBoxed*)obj)->value

RBoxedType r_boxed_get_type (rsexp      obj);
void       r_boxed_set_type (rsexp      obj,
                             RBoxedType type);

#endif  //  __ROSE_DETAIL_BOXED_H__
