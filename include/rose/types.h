#ifndef __ROSE_TYPES_H__
#define __ROSE_TYPES_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#   define R_BEGIN_DECLS    extern "C" {
#   define R_END_DECLS      }
#else
#   define R_BEGIN_DECLS
#   define R_END_DECLS
#endif

R_BEGIN_DECLS

#define FALSE   0
#define TRUE    (!FALSE)
#define UNKNOWN (-1)

typedef int             rint;
typedef unsigned int    ruint;

typedef uint8_t         rbyte;

typedef int8_t          rint8;
typedef uint8_t         ruint8;

typedef int16_t         rint16;
typedef uint16_t        ruint16;

typedef int32_t         rint32;
typedef uint32_t        ruint32;

typedef int64_t         rint64;
typedef uint64_t        ruint64;

typedef void const*     rconstpointer;
typedef void*           rpointer;

typedef char            rchar;
typedef uint32_t        runichar;

typedef rchar*          rcstring;
typedef rchar const*    rconstcstring;

typedef runichar*       runistring;
typedef runichar const* rconstunistring;

typedef int             rbool;
typedef int             rtribool;
typedef uintptr_t       rword;
typedef off_t           roffset;
typedef off64_t         roffset64;
typedef size_t          rsize;
typedef ssize_t         rssize;

#define r_cast(to, from) ((to) (from))

R_END_DECLS

#endif  /* __ROSE_TYPES_H__ */
