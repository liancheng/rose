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

/// \cond
R_BEGIN_DECLS
/// \endcond

/// Boolean false value
#define FALSE 0

/// Boolean true value
#define TRUE (!FALSE)

/// Tri-boolean unknown value
#define UNKNOWN (-1)

/// Signed integer type as wide as a machine word.
typedef intptr_t rintw;

/// Unsigned integer type as wide as a machine word.
typedef uintptr_t ruintw;

/// Unsigned byte type.
typedef uint8_t rbyte;

/// Signed 8-bit integer type.
typedef int8_t rint8;

/// Unsigned 8-bit integer type.
typedef uint8_t ruint8;

/// Signed 16-bit integer type.
typedef int16_t rint16;

/// Unsigned 16-bit integer type.
typedef uint16_t ruint16;

/// Signed 32-bit integer type.
typedef int32_t rint32;

/// Unsigned 32-bit integer type.
typedef uint32_t ruint32;

/// Signed 64-bit integer type.
typedef int64_t rint64;

/// Unsigned 64-bit integer type.
typedef uint64_t ruint64;

/// Const raw pointer type.
typedef void const* rconstpointer;

/// Raw pointer type.
typedef void* rpointer;

/// Signed char type.
typedef char rchar;

/// 32-bit unicode char type.
typedef uint32_t runichar;

/// Null-terminated C-string type.
typedef rchar* rcstring;

/// Const null-terminated C-string type.
typedef rchar const* rconstcstring;

/// Null-terminated unicode string type.
typedef runichar* runistring;

/// Const null-terminated unicode string type.
typedef runichar const* rconstunistring;

/// Boolean type.
typedef int rbool;

/// Tri-boolean type (`TRUE`, `FALSE`, or `UNKNOWN`)
typedef int rtribool;

/// Singed word type.
typedef intptr_t rword;

/// Unsinged word type.
typedef uintptr_t ruword;

/// Singed size type
typedef size_t rsize;

/// Casts expression `from` to type `to`.
#define r_cast(to, from) ((to) (from))

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_TYPES_H__ */
