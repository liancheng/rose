#ifndef __ROSE_DETAIL_PORT_H__
#define __ROSE_DETAIL_PORT_H__

#include "detail/sexp.h"
#include "rose/port.h"

#include <stdio.h>
#include <stdint.h>

R_BEGIN_DECLS

typedef enum RPortMode {
    MODE_INPUT      = 0x01,
    MODE_OUTPUT     = 0x02,
    MODE_BINARY     = 0x04,
    MODE_DONT_CLOSE = 0x08,
    MODE_CLOSED     = 0x10,
    MODE_FOLD_CASE  = 0x20,
    MODE_STRING_IO  = 0x40
}
RPortMode;

typedef void (*RPortClearCookie) (RState*, rpointer);
typedef void (*RPortMarkCookie)  (RState*, rpointer);

FILE* port_to_stream (rsexp port);

R_END_DECLS

#endif  /* __ROSE_DETAIL_PORT_H__ */
