#ifndef __ROSE_STRING_H__
#define __ROSE_STRING_H__

#include "rose/sexp.h"

typedef struct RString RString;

rsexp       r_string_new     (char const* str);
rboolean    r_string_p       (rsexp       obj);
char const* r_string_to_cstr (rsexp       obj);

#endif  //  __ROSE_STRING_H__
