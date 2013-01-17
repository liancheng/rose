#ifndef __ROSE_DETAIL_PRIMITIVE_H__
#define __ROSE_DETAIL_PRIMITIVE_H__

#include "rose/primitive.h"

R_BEGIN_DECLS

typedef struct RPrimitiveDesc RPrimitiveDesc;

struct RPrimitiveDesc {
    rconstcstring name;
    RPrimitiveFunc func;
    rsize required;
    rsize optional;
    rbool rest_p;
};

R_END_DECLS

#endif // __ROSE_DETAIL_PRIMITIVE_H__
