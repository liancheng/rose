#ifndef __ROSE_PORT_H__
#define __ROSE_PORT_H__

#include "rose/context.h"
#include "rose/sexp.h"

#include <stdio.h>

enum {
    INPUT_PORT,
    OUTPUT_PORT,
};

typedef struct RPort {
    FILE* stream;
    rint  mode;
    rsexp name;
}
RPort;

rboolean r_port_p            (rsexp       obj);
rboolean r_input_port_p      (rsexp       obj);
rboolean r_output_port_p     (rsexp       obj);
rsexp    r_open_input_file   (char const* filename);
rsexp    r_open_output_file  (char const* filename);
rsexp    r_stdin_port        ();
rsexp    r_stdout_port       ();
void     r_close_port        (rsexp       port);
rint     r_port_printf       (rsexp       port,
                              char const* format,
                              ...);
char*    r_port_gets         (rsexp       port,
                              char*       dest,
                              rint        size);
rint     r_port_puts         (rsexp       port,
                              char const* str);

#define r_close_input_port(port)\
        r_close_port (port)

#define r_close_output_port(port)\
        r_close_port (port)

#define r_current_input_port(context)\
        r_context_get (context, CTX_CURRENT_INPUT_PORT)

#define r_current_output_port(context)\
        r_context_get (context, CTX_CURRENT_OUTPUT_PORT)

#define r_set_current_input_port_x(port, context)\
        r_context_set_x (context, CTX_CURRENT_INPUT_PORT, port)

#define r_set_current_output_port_x(port, context)\
        r_context_set_x (context, CTX_CURRENT_OUTPUT_PORT, port)

#endif  //  __ROSE_PORT_H__
