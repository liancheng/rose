#ifndef __ROSE_STRING_H__
#define __ROSE_STRING_H__

#include "rose/sexp.h"
#include "rose/state.h"

typedef struct RString RString;

rsexp         r_string_new        (RState*       state,
                                   rconstcstring str);
rbool         r_string_p          (rsexp         obj);
rconstcstring r_string_to_cstr    (rsexp         obj);
rbool         r_string_equal_p    (RState*       state,
                                   rsexp         lhs,
                                   rsexp         rhs);
rint          r_string_byte_count (rsexp         obj);
rint          r_string_length     (rsexp         obj);

#endif  /* __ROSE_STRING_H__ */
