#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include "rose/state.h"
#include "rose/types.h"

typedef rword rsexp;

typedef struct RType RType;

typedef void (*RWriteFunction)   (rsexp, rsexp);
typedef void (*RDisplayFunction) (rsexp, rsexp);

/**
 *  Tagged Pointer Patterns
 *  =======================
 *
 *  Non-immediate types (end with #b00)
 *
 *  - #b000: pointer to boxed heap object
 *  - #b100: pair
 *
 *  Immediate types
 *
 *  - #b001: boolean
 *  - #b010: character
 *  - #b011: even small integer
 *  - #b101: special constant ('(), eof, unspecified, undefined)
 *  - #b110: symbol
 *  - #b111: odd small integer
 */

#define R_TAG_BITS              3
#define R_SMI_BITS              2
#define R_HEAP_OBJ_BITS         2
#define R_TAG_MASK              0x07
#define R_SMI_MASK              0x03
#define R_HEAP_OBJ_MASK         0x03

#define R_BOXED_TAG             0x00
#define R_PAIR_TAG              0x04
#define R_HEAP_OBJ_TAG          0x00

#define R_BOOL_TAG              0x01
#define R_CHAR_TAG              0x02
#define R_SMI_TAG               0x03
#define R_SMI_EVEN_TAG          0x03
#define R_SMI_ODD_TAG           0x07
#define R_SPECIAL_CONST_TAG     0x05
#define R_SYMBOL_TAG            0x06

#define R_MAKE_BOOL(b)          (((b) << R_TAG_BITS) | R_BOOL_TAG)
#define R_FALSE                 (R_MAKE_BOOL (0))
#define R_TRUE                  (R_MAKE_BOOL (1))

#define R_MAKE_SPECIAL_CONST(n) ((((n) << R_TAG_BITS) | R_SPECIAL_CONST_TAG))
#define R_NULL                  (R_MAKE_SPECIAL_CONST (0))
#define R_EOF                   (R_MAKE_SPECIAL_CONST (1))
#define R_UNSPECIFIED           (R_MAKE_SPECIAL_CONST (2))
#define R_UNDEFINED             (R_MAKE_SPECIAL_CONST (3))

#define R_GET_TAG(obj)          ((obj) & R_TAG_MASK)

#define r_boxed_p(obj)          (R_GET_TAG (obj) == R_BOXED_TAG)
#define r_heap_obj_p(obj)       (((obj) & R_HEAP_OBJ_MASK) == R_HEAP_OBJ_TAG)
#define r_immediate_p(obj)      (!(R_HEAP_OBJ_p (obj)))

#define r_bool_p(obj)           (R_GET_TAG (obj) == R_BOOL_TAG)
#define r_false_p(obj)          ((obj) == R_FALSE)
#define r_true_p(obj)           (!r_false_p (obj))

#define r_char_p(obj)           (R_GET_TAG (obj) == R_CHAR_TAG)
#define r_special_const_p(obj)  (R_GET_TAG (obj) == R_SPECIAL_CONST_TAG)
#define r_null_p(obj)           ((obj) == R_NULL)
#define r_eof_object_p(obj)     ((obj) == R_EOF)
#define r_unspecified_p(obj)    ((obj) == R_UNSPECIFIED)
#define r_undefined_p(obj)      ((obj) == R_UNDEFINED)

#define r_bool_to_sexp(b)       ((b) ? R_TRUE : R_FALSE)
#define r_bool_from_sexp(obj)   (r_false_p(obj) ? FALSE : TRUE)
#define r_char_to_sexp(c)       (((c) << R_TAG_BITS) | R_CHAR_TAG)
#define r_char_from_sexp(obj)   ((obj) >> R_TAG_BITS)

#endif  //  __ROSE_SEXP_H__
