#ifndef __ROSE_READ_H__
#define __ROSE_READ_H__

#include "rose/context.h"
#include "rose/sexp.h"

#include <stdio.h>

rsexp r_read_boolean        (FILE*     input,
                             RContext* context);
rsexp r_read_symbol         (FILE*     input,
                             RContext* context);
rsexp r_read_string         (FILE*     input,
                             RContext* context);
rsexp r_read_simple_datum   (FILE*     input,
                             RContext* context);
rsexp r_read_list           (FILE*     input,
                             RContext* context);
rsexp r_read_compound_datum (FILE*     input,
                             RContext* context);
rsexp r_read_datum          (FILE*     input,
                             RContext* context);

#endif  //  __ROSE_READ_H__
