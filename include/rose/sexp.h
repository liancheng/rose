#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include "rose/types.h"

R_BEGIN_DECLS

typedef rword rsexp;

/**
 * \defgroup TaggedPointer Tagged pointer
 * \{
 *
 * \section Tagged Pointer Patterns
 *
 * Rose uses tagged pointers to represent Scheme objects.  This representation
 * is based on the assumption that heap allocated data must be aligned on an
 * 8-byte boundary, which is true for almost all operating systems.  In this
 * case, the lower three bits of the pointer are known to be zero, and we can
 * store type information within these bits.
 *
 * Boxed types:
 *
 * - \c #b000: heap-allocated types, including:
 *
 *   - pair
 *   - string
 *   - vector
 *   - bytevector
 *   - procedure
 *   - continuation
 *   - environment
 *   - port
 *   - error object
 *   - fixnum
 *   - flonum
 *
 * Immediate types:
 *
 * - \c #b010: character (see below)
 * - \c #b011: even small integer (SMI)
 * - \c #b101: special constant (see below)
 * - \c #b110: symbol
 * - \c #b111: odd small integer (SMI)
 */

typedef enum {
    /* Immediate types */
    R_TAG_RESERVED      = 0x01,     /* #b001 */
    R_TAG_CHAR          = 0x02,     /* #b010 */
    R_TAG_SMI           = 0x03,     /* #b011 */
    R_TAG_SMI_EVEN      = 0x03,     /* #b011 */
    R_TAG_SPECIAL_CONST = 0x04,     /* #b100 */
    R_TAG_SYMBOL        = 0x05,     /* #b101 */
    R_TAG_SMI_ODD       = 0x07,     /* #b111 */

    /* Boxed types */
    R_TAG_BOXED         = 0x00,     /* #b000 */
    R_TAG_PAIR          = 0x08,
    R_TAG_STRING        = 0x09,
    R_TAG_VECTOR        = 0x0a,
    R_TAG_BYTEVECTOR    = 0x0b,
    R_TAG_PROCEDURE     = 0x0c,
    R_TAG_PRIMITIVE     = 0x0d,
    R_TAG_ENV           = 0x0e,
    R_TAG_PORT          = 0x0f,
    R_TAG_ERROR         = 0x10,
    R_TAG_FIXNUM        = 0x11,
    R_TAG_FLONUM        = 0x12,
    R_TAG_OPAQUE        = 0x13,

    /* End mark */
    R_TAG_MAX           = 0x14
}
RTypeTag;

typedef struct RObject RObject;

#define R_OBJECT_HEADER\
        RTypeTag type_tag : 5;\
        ruint    gc_color : 2;\
        RObject* gray_next;\
        RObject* chrono_next;

struct RObject {
    R_OBJECT_HEADER
};

#define R_TAG_BITS              3
#define R_TAG_MASK              0x07    /* #b111 */

#define R_SMI_BITS              2
#define R_SMI_MASK              0x03    /* #b011 */

#define r_set_tag_x(obj, tag)   (((obj) << R_TAG_BITS) | (tag))
#define r_get_tag(obj)          ((obj) & R_TAG_MASK)

#define make_special_const(n)   (r_set_tag_x ((n), R_TAG_SPECIAL_CONST))
#define R_FALSE                 (make_special_const (0u))
#define R_TRUE                  (make_special_const (1u))
#define R_NULL                  (make_special_const (2u))
#define R_EOF                   (make_special_const (3u))
#define R_UNSPECIFIED           (make_special_const (4u))
#define R_UNDEFINED             (make_special_const (5u))
#define R_FAILURE               (make_special_const (6u))

#define r_boxed_p(obj)          (r_get_tag (obj) == R_TAG_BOXED)
#define r_immediate_p(obj)      (!(r_boxed_p (obj)))

#define r_bool_p(obj)           ((obj) == R_FALSE || (obj) == R_TRUE)
#define r_false_p(obj)          ((obj) == R_FALSE)
#define r_true_p(obj)           (!r_false_p (obj))

#define r_char_p(obj)           (r_get_tag (obj) == R_TAG_CHAR)
#define r_special_const_p(obj)  (r_get_tag (obj) == R_TAG_SPECIAL_CONST)
#define r_null_p(obj)           ((obj) == R_NULL)
#define r_eof_object_p(obj)     ((obj) == R_EOF)
#define r_unspecified_p(obj)    ((obj) == R_UNSPECIFIED)
#define r_undefined_p(obj)      ((obj) == R_UNDEFINED)

#define r_bool_to_sexp(b)       ((b) ? R_TRUE : R_FALSE)
#define r_bool_from_sexp(obj)   (r_false_p(obj) ? FALSE : TRUE)
#define r_char_to_sexp(c)       (r_cast (rsexp, r_set_tag_x ((c), R_TAG_CHAR)))
#define r_char_from_sexp(obj)   (r_cast (rchar, ((obj) >> R_TAG_BITS)))

#define r_failure_p(obj)        ((obj) == R_FAILURE)

/** \} */

R_END_DECLS

#endif /* __ROSE_SEXP_H__ */
