#ifndef __ROSE_WRITE_H__
#define __ROSE_WRITE_H__

#include "rose/context.h"
#include "rose/sexp.h"

void r_write   (rsexp     port,
                rsexp     obj,
                RContext* context);
void r_display (rsexp     port,
                rsexp     obj,
                RContext* context);

#endif  //  __ROSE_WRITE_H__
