#ifndef __ROSE_STRING_H__
#define __ROSE_STRING_H__

#include "rose/sexp.h"
#include "rose/context.h"

#include <stdio.h>

typedef struct RString {
    rsize length;
    char* data;
}
RString;

rboolean r_string_p     (rsexp       sexp);
rsexp    r_string_new   (char const* str);
void     r_write_string (rsexp       output,
                         rsexp       sexp,
                         rsexp       context);
rsexp    r_read_string  (rsexp       input,
                         rsexp       context);

#endif  //  __ROSE_STRING_H__
