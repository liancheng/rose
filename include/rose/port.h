#ifndef __ROSE_PORT_H__
#define __ROSE_PORT_H__

#include "rose/sexp.h"

#include <stdio.h>

typedef struct RPort {
    FILE*       native_stream;
    char const* name;
}
RPort;

rboolean r_port_p                  (rsexp       sexp);
rsexp    r_file_input_port_new     (FILE*       file,
                                    char const* name,
                                    rboolean    close_on_destroy);
rsexp    r_file_output_port_new    (FILE*       file,
                                    char const* name,
                                    rboolean    close_on_destroy);
void     r_file_port_close         (rsexp       port);
rsexp    r_get_current_input_port  (rsexp       context);
rsexp    r_get_current_output_port (rsexp       context);
void     r_set_current_input_port  (rsexp       port,
                                    rsexp       context);
void     r_set_current_output_port (rsexp       port,
                                    rsexp       context);
rint     r_pprintf                 (rsexp       port,
                                    char const* format,
                                    ...);
char*    r_pgets                   (char*       dest,
                                    rint        size,
                                    rsexp       port);

#endif  //  __ROSE_PORT_H__
