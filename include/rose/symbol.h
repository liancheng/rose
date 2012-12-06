#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/sexp.h"

#define r_symbol_p(obj)     (r_get_tag (obj) == R_SYMBOL_TAG)

rsexp         r_symbol_new        (RState*       state,
                                   rconstcstring symbol);
rsexp         r_symbol_new_static (RState*       state,
                                   rconstcstring symbol);
rconstcstring r_symbol_name       (RState*       state,
                                   rsexp         obj);

#endif  /* __ROSE_SYMBOL_H__ */
