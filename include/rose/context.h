#ifndef __ROSE_CONTEXT_H__
#define __ROSE_CONTEXT_H__

#include "rose/sexp.h"

typedef enum {
    CTX_SCANNER,
    CTX_SYMBOL_TABLE,
    CTX_ENV,
    CTX_CURRENT_INPUT_PORT,
    CTX_CURRENT_OUTPUT_PORT,
    CTX_N_FIELD
}
RContextField;

rsexp r_context_new   ();
rsexp r_context_get   (rsexp context,
                       ruint key);
rsexp r_context_set_x (rsexp context,
                       ruint key,
                       rsexp value);

#endif  //  __ROSE_CONTEXT_H__
