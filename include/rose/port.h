#ifndef __ROSE_PORT_H__
#define __ROSE_PORT_H__

#include "rose/sexp.h"

typedef struct RPort RPort;

rsexp    r_open_input_file           (char const* filename);
rsexp    r_open_output_file          (char const* filename);
rsexp    r_open_input_string         (char const* string);
rsexp    r_stdin_port                ();
rsexp    r_stdout_port               ();

void     r_close_input_port          (rsexp       port);
void     r_close_output_port         (rsexp       port);
void     r_close_port                (rsexp       port);

rboolean r_eof_p                     (rsexp       port);
rboolean r_port_p                    (rsexp       obj);
rboolean r_input_port_p              (rsexp       obj);
rboolean r_output_port_p             (rsexp       obj);

rint     r_port_printf               (rsexp       port,
                                      char const* format,
                                      ...);
char*    r_port_gets                 (rsexp       port,
                                      char*       dest,
                                      rint        size);
rint     r_port_puts                 (rsexp       port,
                                      char const* str);
void     r_write_char                (rsexp       port,
                                      char        ch);

rsexp    r_current_input_port        (RContext*   context);
rsexp    r_current_output_port       (RContext*   context);
void     r_set_current_input_port_x  (rsexp       port,
                                      RContext*   context);
void     r_set_current_output_port_x (rsexp       port,
                                      RContext*   context);

#endif  //  __ROSE_PORT_H__
