#ifndef __ROSE_PORT_H__
#define __ROSE_PORT_H__

#include "rose/sexp.h"

#include <stdio.h>

typedef struct RPort {
    FILE*    stream;
    char*    name;
    rboolean is_open;
    rboolean close_on_destroy;
}
RPort;

rboolean r_port_p (rsexp sexp);

#endif  //  __ROSE_PORT_H__
