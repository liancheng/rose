#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

#define r_symbol_p(obj)         (r_get_tag (obj) == R_TAG_SYMBOL)
#define r_intern(r, str)        (r_symbol_new (r, str))
#define r_intern_static(r, str) (r_symbol_new_static (r, str))

rsexp         r_symbol_new        (RState* r,
                                   rconstcstring symbol);
rsexp         r_symbol_new_static (RState* r,
                                   rconstcstring symbol);
rconstcstring r_symbol_name       (RState* r,
                                   rsexp obj);

R_END_DECLS

#endif /* __ROSE_SYMBOL_H__ */
