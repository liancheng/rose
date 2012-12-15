#ifndef __ROSE_DETAIL_ERROR_H__
#define __ROSE_DETAIL_ERROR_H__

#include "rose/error.h"

rsexp error_wrong_type_arg (RState*       state,
                            rconstcstring expected,
                            rsexp         given);

#endif  //  __ROSE_DETAIL_ERROR_H__
