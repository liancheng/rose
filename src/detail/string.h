#ifndef __ROSE_DETAIL_STRING_H__
#define __ROSE_DETAIL_STRING_H__

#include "rose/sexp.h"
#include "rose/string.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

typedef struct RString RString;

struct RString {
    R_OBJECT_HEADER
    rsize length;
    rcstring data;
};

#define string_from_sexp(obj)   (r_cast (RString*, (obj)))
#define string_to_sexp(string)  (r_cast (rsexp, (string)))

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_DETAIL_STRING_H__ */
