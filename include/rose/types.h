#ifndef __ROSE_TYPES_H__
#define __ROSE_TYPES_H__

#include <stdint.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE  (!FALSE)

typedef int          rint;
typedef unsigned int ruint;
typedef uintptr_t    rword;
typedef void*        rpointer;
typedef void const*  rconstpointer;
typedef size_t       rsize;
typedef int          rboolean;

#endif  //  __ROSE_TYPES_H__
