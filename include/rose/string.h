#ifndef __ROSE_STRING_H__
#define __ROSE_STRING_H__

#include "rose/sexp.h"

#define SEXP_STRING_P(s) SEXP_CHECK_TYPE(s, SEXP_STRING)

rsexp sexp_string_strdup(char const* str);

#endif  //  __ROSE_STRING_H__
