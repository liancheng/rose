#ifndef __ROSE_WRITE_H__
#define __ROSE_WRITE_H__

#include "rose/sexp.h"

void r_write   (rsexp port, rsexp obj);
void r_display (rsexp port, rsexp obj);

#endif  /* __ROSE_WRITE_H__ */
