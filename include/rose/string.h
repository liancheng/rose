#ifndef __ROSE_STRING_H__
#define __ROSE_STRING_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef struct RString RString;

rsexp        r_string_new     (RState*      state,
                               rchar const* str);
rbool        r_string_p       (rsexp        obj);
rchar const* r_string_to_cstr (rsexp        obj);
rbool        r_string_equal_p (RState*      state,
                               rsexp        lhs,
                               rsexp        rhs);

#endif  /* __ROSE_STRING_H__ */
