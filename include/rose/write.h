#ifndef __ROSE_WRITE_H__
#define __ROSE_WRITE_H__

#include "rose/context.h"
#include "rose/sexp.h"

#include <stdio.h>

void r_write_datum  (FILE*     output,
                     rsexp     sexp,
                     RContext* context);
void r_write_pair   (FILE*     output,
                     rsexp     sexp,
                     RContext* context);
void r_write_cdr    (FILE*     output,
                     rsexp     sexp,
                     RContext* context);
void r_write_vector (FILE*     output,
                     rsexp     sexp,
                     RContext* context);

#endif  //  __ROSE_WRITE_H__
