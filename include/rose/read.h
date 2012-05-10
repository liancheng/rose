#ifndef __ROSE_READ_H__
#define __ROSE_READ_H__

#include "rose/sexp.h"

#define READ_EXPECT TRUE
#define READ_TRY    FALSE

rsexp r_read (rsexp    port,
              rboolean expect,
              rsexp    context);

#endif  //  __ROSE_READ_H__
