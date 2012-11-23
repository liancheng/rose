#ifndef __ROSE_DETAIL_PORT_H__
#define __ROSE_DETAIL_PORT_H__

#include "detail/sexp.h"
#include "rose/state.h"

#include <stdio.h>
#include <stdint.h>

enum {
    INPUT_PORT,
    OUTPUT_PORT
};

typedef rint (*RPortReadFunction)  (rpointer, rbyte*, rsize);
typedef rint (*RPortWriteFunction) (rpointer, rbyte*, rsize);
typedef rint (*RPortSeekFunction)  (rpointer, rint64, rint);
typedef rint (*RPortCloseFunction) (rpointer);

typedef struct RPortIOFunctions {
    RPortReadFunction  read;
    RPortWriteFunction write;
    RPortSeekFunction  seek;
    RPortCloseFunction close;
}
RPortIOFunctions;

struct RPort {
    R_OBJECT_HEADER
    RState*          state;
    rint             mode;
    rsexp            name;
    rbool            closed;
    rbool            fold_case;
    RPortIOFunctions io;
    rpointer         cookie;
    FILE*            stream;
};

#define PORT_FROM_SEXP(obj) ((RPort*) (obj))
#define PORT_TO_SEXP(port)  ((rsexp) (port))
#define PORT_TO_FILE(obj)   ((FILE*) (PORT_FROM_SEXP (obj)->stream))

#endif  /* __ROSE_DETAIL_PORT_H__ */
