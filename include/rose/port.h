#ifndef __ROSE_PORT_H__
#define __ROSE_PORT_H__

#include "rose/sexp.h"

typedef struct RPort RPort;

RState* r_port_get_state            (rsexp       port);
rsexp   r_open_input_file           (RState*     state,
                                     char const* filename);
rsexp   r_open_output_file          (RState*     state,
                                     char const* filename);
rsexp   r_open_input_string         (RState*     state,
                                     char const* string);
rsexp   r_stdin_port                (RState*     state);
rsexp   r_stdout_port               (RState*     state);

rsexp   r_port_get_name             (rsexp       port);

void    r_close_input_port          (rsexp       port);
void    r_close_output_port         (rsexp       port);
void    r_close_port                (rsexp       port);

rbool   r_eof_p                     (rsexp       port);
rbool   r_port_p                    (rsexp       obj);
rbool   r_input_port_p              (rsexp       obj);
rbool   r_output_port_p             (rsexp       obj);

rint    r_port_printf               (rsexp       port,
                                     char const* format,
                                     ...);
char*   r_port_gets                 (rsexp       port,
                                     char*       dest,
                                     rint        size);
rint    r_port_puts                 (rsexp       port,
                                     char const* str);
void    r_write_char                (rsexp       port,
                                     char        ch);
void    r_format                    (rsexp       port,
                                     char const* format,
                                     ...);

rsexp   r_current_input_port        (RState*     state);
rsexp   r_current_output_port       (RState*     state);
void    r_set_current_input_port_x  (RState*     state,
                                     rsexp       port);
void    r_set_current_output_port_x (RState*     state,
                                     rsexp       port);

#endif  //  __ROSE_PORT_H__
