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
 * Non-immediate types (end with \c #b00):
 *
 * - \c #b100: pair
 * - \c #b000: all heap-allocated types other than pair
 *
 * Immediate types:
 *
 * - \c #b001: inline error
 * - \c #b010: character (see below)
 * - \c #b011: even small integer (SMI)
 * - \c #b101: special constant (see below)
 * - \c #b110: symbol
 * - \c #b111: odd small integer (SMI)
 *
 * Here is a list of terms about Rose types:
 *
 * - Immediate type:
 *
 *   All types whose value can fit into one \c sexp (a single machine word).
 *   Their type tag are none-zero.  Immediate types include:
 *
 *   - inline error
 *   - character
 *   - small integer
 *   - special constant
 *   - symbol
 *
 * - Boxed type:
 *
 *   All types that are allocated on heap, except pair.  Their type tags are
 *   zero, which means the \c sexp is exactly the pointer to those heap
 *   allocated objects.  Boxed types include:
 *
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
 * - Tagged type:
 *
 *   All types whose type tag is none-zero, i.e. immediate types and pair.
 *
 * - Non-immediate type:
 *
 *   All types that are not immediate type, i.e., all boxed types plus pair.
 */

typedef enum {
    /* Tagged types */
    R_INLINE_ERROR_TAG  = 0x01,     /* #b001 */
    R_CHAR_TAG          = 0x02,     /* #b010 */
    R_PAIR_TAG          = 0x04,     /* #b100 */
    R_SPECIAL_CONST_TAG = 0x05,     /* #b101 */
    R_SYMBOL_TAG        = 0x06,     /* #b110 */

    R_SMI_TAG           = 0x03,     /* #b011 */
    R_SMI_EVEN_TAG      = 0x03,     /* #b011 */
    R_SMI_ODD_TAG       = 0x07,     /* #b111 */

    /* Boxed types */
    R_BOXED_TAG         = 0x00,     /* #b000 */
    R_STRING_TAG        = 0x08,
    R_VECTOR_TAG        = 0x09,
    R_BYTEVECTOR_TAG    = 0x0a,
    R_PROCEDURE_TAG     = 0x0b,
    R_CONTINUATION_TAG  = 0x0c,
    R_ENV_TAG           = 0x0d,
    R_PORT_TAG          = 0x0e,
    R_ERROR_TAG         = 0x0f,
    R_FIXNUM_TAG        = 0x10,
    R_FLONUM_TAG        = 0x11,

    /* End mark */
    R_TAG_MAX           = 0x12
}
RTypeTag;

#define R_TAG_BITS              3
#define R_TAG_MASK              0x07    /* #b111 */

#define R_SMI_BITS              2
#define R_SMI_MASK              0x03    /* #b011 */

#define __SPECIAL_CONST(n)      (((n) << R_TAG_BITS) | R_SPECIAL_CONST_TAG)
#define R_FALSE                 (__SPECIAL_CONST (0))
#define R_TRUE                  (__SPECIAL_CONST (1))
#define R_NULL                  (__SPECIAL_CONST (2))
#define R_EOF                   (__SPECIAL_CONST (3))
#define R_UNSPECIFIED           (__SPECIAL_CONST (4))
#define R_UNDEFINED             (__SPECIAL_CONST (5))
#define R_FAILURE               (__SPECIAL_CONST (6))

#define r_get_tag(obj)          ((obj) & R_TAG_MASK)

#define r_boxed_p(obj)          (r_get_tag (obj) == R_BOXED_TAG)
#define r_pair_p(obj)           (r_get_tag (obj) == R_PAIR_TAG)
#define r_immediate_p(obj)      (!(r_boxed_p (obj)) && !(r_pair_p (obj)))
#define r_tagged_p(obj)         (!(r_boxed_p (obj)))

#define r_bool_p(obj)           ((obj) == R_FALSE || (obj) == R_TRUE)
#define r_false_p(obj)          ((obj) == R_FALSE)
#define r_true_p(obj)           (!r_false_p (obj))

#define r_char_p(obj)           (r_get_tag (obj) == R_CHAR_TAG)
#define r_special_const_p(obj)  (r_get_tag (obj) == R_SPECIAL_CONST_TAG)
#define r_null_p(obj)           ((obj) == R_NULL)
#define r_eof_object_p(obj)     ((obj) == R_EOF)
#define r_unspecified_p(obj)    ((obj) == R_UNSPECIFIED)
#define r_undefined_p(obj)      ((obj) == R_UNDEFINED)

#define r_bool_to_sexp(b)       ((b) ? R_TRUE : R_FALSE)
#define r_bool_from_sexp(obj)   (r_false_p(obj) ? FALSE : TRUE)
#define r_char_to_sexp(c)       (((c) << R_TAG_BITS) | R_CHAR_TAG)
#define r_char_from_sexp(obj)   ((obj) >> R_TAG_BITS)

#define r_failure_p(obj)        ((obj) == R_FAILURE)

/** \} */

R_END_DECLS

#endif  /* __ROSE_SEXP_H__ */
