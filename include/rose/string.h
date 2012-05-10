#ifndef __ROSE_STRING_H__
#define __ROSE_STRING_H__

#include "rose/sexp.h"

typedef struct RString {
    rsize length;
    char* data;
}
RString;

char*       r_strdup      (char const* str);
rboolean    r_string_p    (rsexp       obj);
rsexp       r_string_new  (char const* str);
char const* r_string_cstr (rsexp       obj);

#endif  //  __ROSE_STRING_H__
