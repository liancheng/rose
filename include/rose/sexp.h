#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include "rose/types.h"

typedef enum {
    SEXP_ENV,
    SEXP_STRING,
    SEXP_VECTOR,
    SEXP_PORT,
    SEXP_ERROR,
    SEXP_OPAQUE,
}
RCellType;

typedef rword rsexp;

/*
 * Simple tagging.  Ends in:
 *
 * - #b000 (#x0, #d0): pointer to cell object
 * - #b001 (#x1, #d1): small fixnum
 * - #b010 (#x2, #d2): pair
 * - #b011 (#x3, #d3): char
 * - #b100 (#x3, #d3): symbol
 * - #b111 (#x7, #d7): immediate constants ('(), #t, #f, etc.)
 */
#define R_SEXP_TAG_MASK             0x07
#define R_SEXP_CELL_TAG             0x00
#define R_SEXP_SMALL_FIXNUM_TAG     0x01
#define R_SEXP_PAIR_TAG             0x02
#define R_SEXP_SYMBOL_TAG           0x03
#define R_SEXP_IMMEDIATE_TAG        0x07

#define R_SEXP_MAKE_IMMEDIATE(n)    ((rsexp) ((n << 3) | R_SEXP_IMMEDIATE_TAG))
#define R_SEXP_NULL                 R_SEXP_MAKE_IMMEDIATE (0)
#define R_SEXP_FALSE                R_SEXP_MAKE_IMMEDIATE (1)
#define R_SEXP_TRUE                 R_SEXP_MAKE_IMMEDIATE (2)
#define R_SEXP_EOF                  R_SEXP_MAKE_IMMEDIATE (3)
#define R_SEXP_UNSPECIFIED          R_SEXP_MAKE_IMMEDIATE (4)
#define R_SEXP_UNDEFINED            R_SEXP_MAKE_IMMEDIATE (5)

#define r_null_p(obj)           ((obj) == R_SEXP_NULL)
#define r_boolean_p(obj)        ((obj) == R_SEXP_TRUE || (obj) == R_SEXP_FALSE)
#define r_false_p(obj)          ((obj) == R_SEXP_FALSE)
#define r_true_p(obj)           ((obj) != R_SEXP_FALSE)
#define r_eof_object_p(obj)     ((obj) == R_SEXP_EOF)
#define r_unspecified_p(obj)    ((obj) == R_SEXP_UNSPECIFIED)
#define r_undefined_p(obj)      ((obj) == R_SEXP_UNDEFINED)

#include <gc/gc.h>

#define R_SEXP_NEW(obj, type)\
        rsexp obj = (rsexp) GC_NEW (RCell);\
        r_cell_set_type (obj, type)

#endif  //  __ROSE_SEXP_H__
