#ifndef __ROSE_READ_H__
#define __ROSE_READ_H__

#include "rose/context.h"
#include "rose/sexp.h"

#include <stdio.h>

r_sexp sexp_read_boolean        (FILE*      input,
                                 r_context* context);
r_sexp sexp_read_symbol         (FILE*      input,
                                 r_context* context);
r_sexp sexp_read_string         (FILE*      input,
                                 r_context* context);
r_sexp sexp_read_simple_datum   (FILE*      input,
                                 r_context* context);
r_sexp sexp_read_list           (FILE*      input,
                                 r_context* context);
r_sexp sexp_read_compound_datum (FILE*      input,
                                 r_context* context);
r_sexp sexp_read_datum          (FILE*      input,
                                 r_context* context);

#endif  //  __ROSE_READ_H__
