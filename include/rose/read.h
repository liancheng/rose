#ifndef __ROSE_READ_H__
#define __ROSE_READ_H__

#include "rose/context.h"
#include "rose/sexp.h"

#include <stdio.h>

rsexp r_read (FILE*     input,
              RContext* context);

#endif  //  __ROSE_READ_H__
