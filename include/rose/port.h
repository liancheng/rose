#ifndef __ROSE_PORT_H__
#define __ROSE_PORT_H__

#include "rose/sexp.h"
#include "rose/state.h"

#include <stdarg.h>

R_BEGIN_DECLS

rsexp    r_open_input_file           (RState* r,
                                      rconstcstring filename);
rsexp    r_open_output_file          (RState* r,
                                      rconstcstring filename);
rsexp    r_open_input_string         (RState* r,
                                      rsexp string);
rsexp    r_open_output_string        (RState* r);
rsexp    r_get_output_string         (RState* r,
                                      rsexp port);
rsexp    r_stdin_port                (RState* r);
rsexp    r_stdout_port               (RState* r);
rsexp    r_stderr_port               (RState* r);
rsexp    r_port_get_name             (rsexp port);
void     r_close_port                (rsexp port);
rbool    r_eof_p                     (rsexp port);
rbool    r_port_p                    (rsexp obj);
rbool    r_input_port_p              (rsexp obj);
rbool    r_output_port_p             (rsexp obj);
rsexp    r_port_vprintf              (RState* r,
                                      rsexp port,
                                      rconstcstring format,
                                      va_list args);
rsexp    r_port_printf               (RState*       r,
                                      rsexp port,
                                      rconstcstring format,
                                      ...);
rsexp    r_printf                    (RState* r,
                                      rconstcstring format,
                                      ...);
rcstring r_port_gets                 (RState* r,
                                      rsexp port,
                                      rcstring dest,
                                      rint size);
rsexp    r_port_puts                 (RState* r,
                                      rsexp port,
                                      rconstcstring str);
rsexp    r_port_read_char            (RState* r,
                                      rsexp port);
rsexp    r_read_char                 (RState* r);
rsexp    r_port_write_char           (RState* r,
                                      rsexp port,
                                      rchar ch);
rsexp    r_write_char                (RState* r,
                                      rchar ch);
rsexp    r_port_vformat              (RState* r,
                                      rsexp port,
                                      rconstcstring format,
                                      va_list args);
rsexp    r_port_format               (RState* r,
                                      rsexp port,
                                      rconstcstring format,
                                      ...);
rsexp    r_format                    (RState* r,
                                      rconstcstring format,
                                      ...);
rsexp    r_current_input_port        (RState* r);
rsexp    r_current_output_port       (RState* r);
rsexp    r_current_error_port        (RState* r);
void     r_set_current_input_port_x  (RState* r,
                                      rsexp port);
void     r_set_current_output_port_x (RState* r,
                                      rsexp port);
void     r_set_current_error_port_x  (RState* r,
                                      rsexp port);
rsexp    r_port_write                (RState* r,
                                      rsexp port,
                                      rsexp obj);
rsexp    r_write                     (RState* r,
                                      rsexp obj);
rsexp    r_port_display              (RState* r,
                                      rsexp port,
                                      rsexp obj);
rsexp    r_display                   (RState* r,
                                      rsexp obj);

R_END_DECLS

#endif  /* __ROSE_PORT_H__ */
