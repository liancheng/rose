#ifndef __ROSE_ERROR_H__
#define __ROSE_ERROR_H__

#include "rose/sexp.h"

#include <stdarg.h>

#define SEXP_ERROR_P(s) SEXP_CHECK_TYPE(s, SEXP_ERROR)

rsexp r_error                  (rsexp message,
                                rsexp irritants);
rsexp r_error_object_message   (rsexp error);
rsexp r_error_object_irritants (rsexp error);

#endif  //  __ROSE_ERROR_H__
