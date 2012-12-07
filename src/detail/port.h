#ifndef __ROSE_DETAIL_PORT_H__
#define __ROSE_DETAIL_PORT_H__

#include "detail/sexp.h"
#include "rose/port.h"

#include <stdio.h>
#include <stdint.h>

typedef enum RPortMode {
    MODE_INPUT      = 0x01,
    MODE_OUTPUT     = 0x02,
    MODE_BINARY     = 0x04,
    MODE_DONT_CLOSE = 0x08,
    MODE_CLOSED     = 0x10,
    MODE_FOLD_CASE  = 0x20,
    MODE_STRING_IO  = 0x40,
    MODE_FLUSH      = 0x80
}
RPortMode;

typedef void (*RPortClearFunc) (RState*, rpointer);
typedef void (*RPortMarkFunc)  (RState*, rpointer);

struct RPort {
    R_OBJECT_HEADER

    RState*        state;
    FILE*          stream;
    rsexp          name;
    RPortMode      mode;

    rpointer       cookie;
    RPortClearFunc clear;
    RPortMarkFunc  mark;
};

#define port_from_sexp(obj) (r_cast (RPort*, (obj)))
#define port_to_sexp(port)  (r_cast (rsexp, (port)))
#define port_to_file(obj)   (port_from_sexp (obj)->stream)

void init_port_type_info (RState* state);

#endif  /* __ROSE_DETAIL_PORT_H__ */
