#ifndef __ROSE_ERROR_H__
#define __ROSE_ERROR_H__

#include "rose/sexp.h"

#include <stdarg.h>

typedef struct RError {
    rsexp message;
    rsexp irritants;
}
RError;

rboolean r_error_p               (rsexp obj);
rsexp    r_error                 (rsexp message,
                                  rsexp irritants);
rsexp    r_error_message         (rsexp error);
rsexp    r_error_irritants       (rsexp error);
void     r_error_set_message_x   (rsexp error,
                                  rsexp message);
void     r_error_set_irritants_x (rsexp error,
                                  rsexp irritants);

#endif  //  __ROSE_ERROR_H__
