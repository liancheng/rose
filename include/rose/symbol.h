#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/sexp.h"

R_BEGIN_DECLS

#define r_symbol_p(obj)     (r_get_tag (obj) == R_TAG_SYMBOL)

rsexp         r_symbol_new        (RState*       state,
                                   rconstcstring symbol);
rsexp         r_symbol_new_static (RState*       state,
                                   rconstcstring symbol);
rconstcstring r_symbol_name       (RState*       state,
                                   rsexp         obj);

R_END_DECLS

#endif  /* __ROSE_SYMBOL_H__ */
