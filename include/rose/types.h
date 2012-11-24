#ifndef __ROSE_TYPES_H__
#define __ROSE_TYPES_H__

#include <stdint.h>
#include <stdlib.h>

#define FALSE   0
#define TRUE    (!FALSE)
#define UNKNOWN (-1)

typedef char         rchar;
typedef int16_t      rint16;
typedef int32_t      rint32;
typedef int64_t      rint64;
typedef int8_t       rint8;
typedef int          rbool;
typedef int          rint;
typedef int          rtribool;
typedef size_t       rsize;
typedef uint16_t     ruint16;
typedef uint32_t     ruint32;
typedef uint32_t     runichar;
typedef uint64_t     ruint64;
typedef uint8_t      rbyte;
typedef uint8_t      ruint8;
typedef uintptr_t    rword;
typedef unsigned int ruint;
typedef void const*  rconstpointer;
typedef void*        rpointer;

#define r_cast(to, from) ((to) (from))

#endif  /* __ROSE_TYPES_H__ */
