#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include "rose/types.h"

typedef enum {
    SEXP_ENV,
    SEXP_STRING,
    SEXP_VECTOR,
    SEXP_PORT,
    SEXP_ERROR,
}
RCellType;

typedef rword rsexp;

/*
 * Simple tagging.  Ends in:
 *
 * - #b000: pointer to cell object
 * - #b001: int30 (even)
 * - #b010: pair
 * - #b011: fixnum
 * - #b100: non-int30 immediate objects
 *   - #b00100: special constants ('(), #t, #f, eof, unspecified, undefined)
 *   - #b01100: characters
 *   - #b10100: symbols
 * - #b101: int30 (odd)
 * - #b110: boolean
 * - #b111: flonum
 */

#define R_TC3_MASK              0x07
#define R_TC5_MASK              0x1f
#define R_TC3_BITS              3
#define R_TC5_BITS              5

#define R_CELL_TAG              0x00
#define R_INT30_TAG             0x01
#define R_PAIR_TAG              0x02
#define R_FIXNUM_TAG            0x03
#define R_BOOLEAN_TAG           0x06
#define R_FLONUM_TAG            0x07

#define R_TC5_TAG               0x04
#define R_SPECIAL_CONST_TAG     0x04
#define R_CHARACTER_TAG         0x0a
#define R_SYMBOL_TAG            0x14

#define R_MAKE_BOOLEAN(n)       ((n << R_TC3_BITS) | R_BOOLEAN_TAG)
#define R_FALSE                 R_MAKE_BOOLEAN(0)
#define R_TRUE                  R_MAKE_BOOLEAN(1)

#define R_MAKE_SPECIAL_CONST(n) ((((n) << R_TC5_BITS) | R_SPECIAL_CONST_TAG))
#define R_NULL                  R_MAKE_SPECIAL_CONST (0)
#define R_EOF                   R_MAKE_SPECIAL_CONST (3)
#define R_UNSPECIFIED           R_MAKE_SPECIAL_CONST (4)
#define R_UNDEFINED             R_MAKE_SPECIAL_CONST (5)

#define r_boolean_p(obj)        (((obj) & R_TC3_MASK) == R_BOOLEAN_TAG)
#define r_special_const_p(obj)  (((obj) & R_TC5_MASK) == R_SPECIAL_CONST_TAG)
#define r_null_p(obj)           ((obj) == R_NULL)
#define r_false_p(obj)          ((obj) == R_FALSE)
#define r_true_p(obj)           ((obj) != R_FALSE)
#define r_eof_object_p(obj)     ((obj) == R_EOF)
#define r_unspecified_p(obj)    ((obj) == R_UNSPECIFIED)
#define r_undefined_p(obj)      ((obj) == R_UNDEFINED)

#define r_int_to_sexp(n)        ((rsexp) (((n) << 2) | R_INT30_TAG))
#define r_int_from_sexp(obj)    (((int) (obj)) >> 2)
#define r_bool_to_sexp(b)       ((b) ? R_TRUE : R_FALSE)
#define r_bool_from_sexp(obj)   (r_false_p(obj) ? FALSE : TRUE)

#endif  //  __ROSE_SEXP_H__
