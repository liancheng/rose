#ifndef __ROSE_PORT_H__
#define __ROSE_PORT_H__

#include "rose/sexp.h"
#include "rose/state.h"

#include <stdarg.h>

R_BEGIN_DECLS

typedef struct RPort RPort;

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
rsexp    r_port_vprintf              (RState*       state,
                                      rsexp         port,
                                      rconstcstring format,
                                      va_list       args);
rsexp    r_port_printf               (RState*       state,
                                      rsexp         port,
                                      rconstcstring format,
                                      ...);
rcstring r_port_gets                 (RState*       state,
                                      rsexp         port,
                                      rcstring      dest,
                                      rint          size);
rsexp    r_port_puts                 (RState*       state,
                                      rsexp         port,
                                      rconstcstring str);
rsexp    r_port_read_char            (RState*       state,
                                      rsexp         port);
rsexp    r_read_char                 (RState*       state);
rsexp    r_port_write_char           (RState*       state,
                                      rsexp         port,
                                      rchar         ch);
rsexp    r_write_char                (RState*       state,
                                      rchar         ch);
rsexp    r_port_vformat              (RState*       state,
                                      rsexp         port,
                                      rconstcstring format,
                                      va_list       args);
rsexp    r_port_format               (RState*       state,
                                      rsexp         port,
                                      rconstcstring format,
                                      ...);
rsexp    r_format                    (RState*       state,
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
rsexp    r_port_write                (RState*       state,
                                      rsexp         port,
                                      rsexp         obj);
rsexp    r_write                     (RState*       state,
                                      rsexp         obj);
rsexp    r_port_display              (RState*       state,
                                      rsexp         port,
                                      rsexp         obj);
rsexp    r_display                   (RState*       state,
                                      rsexp         obj);

R_END_DECLS

#endif  /* __ROSE_PORT_H__ */
