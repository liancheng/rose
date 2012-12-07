#ifndef __ROSE_ERROR_H__
#define __ROSE_ERROR_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

#define R_ERROR_OOM (__R_SPECIAL_CONST (4))

typedef struct RError RError;

rsexp r_error_new             (RState*       state,
                               rsexp         message,
                               rsexp         irritants);
rbool r_error_p               (rsexp         obj);
rsexp r_error_get_message     (rsexp         error);
rsexp r_error_get_irritants   (rsexp         error);
void  r_error_set_message_x   (rsexp         error,
                               rsexp         message);
void  r_error_set_irritants_x (rsexp         error,
                               rsexp         irritants);
void  r_error                 (RState*       state,
                               rconstcstring message);

R_END_DECLS

#endif  /* __ROSE_ERROR_H__ */
