#ifndef __ROSE_CONTEXT_H__
#define __ROSE_CONTEXT_H__

#include "rose/sexp.h"

typedef enum {
    CTX_SCANNER,
    CTX_SYMBOL_TABLE,
    CTX_ENV,
    CTX_KEYWORDS,
    CTX_N_FIELD
}
RContextField;

rsexp r_context_new   ();
rsexp r_context_field (rsexp context,
                       rint  name);

#endif  //  __ROSE_CONTEXT_H__
