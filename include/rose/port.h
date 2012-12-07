#ifndef __ROSE_PORT_H__
#define __ROSE_PORT_H__

#include "rose/sexp.h"
#include "rose/state.h"

#include <stdarg.h>

R_BEGIN_DECLS

typedef struct RPort RPort;

RState*  r_port_get_state            (rsexp         port);
rsexp    r_open_input_file           (RState*       state,
                                      rconstcstring filename);
rsexp    r_open_output_file          (RState*       state,
                                      rconstcstring filename);
rsexp    r_open_input_string         (RState*       state,
                                      rsexp         string);
rsexp    r_open_output_string        (RState*       state);
rsexp    r_get_output_string         (RState*       state,
                                      rsexp         port);
rsexp    r_stdin_port                (RState*       state);
rsexp    r_stdout_port               (RState*       state);
rsexp    r_stderr_port               (RState*       state);
rsexp    r_port_get_name             (rsexp         port);
void     r_close_port                (rsexp         port);
rbool    r_eof_p                     (rsexp         port);
rbool    r_port_p                    (rsexp         obj);
rbool    r_input_port_p              (rsexp         obj);
rbool    r_output_port_p             (rsexp         obj);
rint     r_port_vprintf              (rsexp         port,
                                      rconstcstring format,
                                      va_list       args);
rint     r_port_printf               (rsexp         port,
                                      rconstcstring format,
                                      ...);
rcstring r_port_gets                 (rsexp         port,
                                      rcstring      dest,
                                      rint          size);
rint     r_port_puts                 (rsexp         port,
                                      rconstcstring str);
rchar    r_read_char                 (rsexp         port);
void     r_write_char                (rsexp         port,
                                      rchar         ch);
void     r_port_vformat              (RState*       state,
                                      rsexp         port,
                                      rconstcstring format,
                                      va_list       args);
void     r_port_format               (RState*       state,
                                      rsexp         port,
                                      rconstcstring format,
                                      ...);
void     r_format                    (RState*       state,
                                      rconstcstring format,
                                      ...);
rsexp    r_current_input_port        (RState*       state);
rsexp    r_current_output_port       (RState*       state);
rsexp    r_current_error_port        (RState*       state);
void     r_set_current_input_port_x  (RState*       state,
                                      rsexp         port);
void     r_set_current_output_port_x (RState*       state,
                                      rsexp         port);
void     r_set_current_error_port_x  (RState*       state,
                                      rsexp         port);
void     r_port_write                (RState*       state,
                                      rsexp         port,
                                      rsexp         obj);
void     r_write                     (RState*       state,
                                      rsexp         obj);
void     r_port_display              (RState*       state,
                                      rsexp         port,
                                      rsexp         obj);
void     r_display                   (RState*       state,
                                      rsexp         obj);

R_END_DECLS

#endif  /* __ROSE_PORT_H__ */
