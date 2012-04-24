#ifndef __ROSE_ERROR_H__
#define __ROSE_ERROR_H__

#include "rose/sexp.h"

#include <stdarg.h>

#define SEXP_ERROR_P(s) SEXP_CHECK_TYPE(s, SEXP_ERROR)

r_sexp sexp_error                  (r_sexp message,
                                    r_sexp irritants);
r_sexp sexp_error_object_message   (r_sexp error);
r_sexp sexp_error_object_irritants (r_sexp error);

#endif  //  __ROSE_ERROR_H__
