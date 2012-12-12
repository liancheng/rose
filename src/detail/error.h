#ifndef __ROSE_DETAIL_ERROR_H__
#define __ROSE_DETAIL_ERROR_H__

#include "detail/sexp.h"
#include "rose/error.h"

void  init_error_type_info (RState*       state);
rsexp error_wrong_type_arg (RState*       state,
                            rconstcstring expected,
                            rsexp         given);

#endif  //  __ROSE_DETAIL_ERROR_H__
