#ifndef __ROSE_ERROR_H__
#define __ROSE_ERROR_H__

#include "rose/sexp.h"

#include <stdarg.h>

#define SEXP_ERROR_P(s) SEXP_CHECK_TYPE(s, SEXP_ERROR)

rsexp sexp_error                  (rsexp message,
                                   rsexp irritants);
rsexp sexp_error_object_message   (rsexp error);
rsexp sexp_error_object_irritants (rsexp error);

#endif  //  __ROSE_ERROR_H__
