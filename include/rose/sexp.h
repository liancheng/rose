#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include "rose/types.h"

#include <gc/gc.h>

typedef enum {
    SEXP_ENV,
    SEXP_STRING,
    SEXP_VECTOR,
    SEXP_PORT,
    SEXP_ERROR,
    SEXP_OPAQUE,
}
RTypes;

typedef rword rsexp;

/*
 * Simple tagging.  Ends in:
 *
 * - #b000 (#x0, #d0): pointer to boxed object
 * - #b001 (#x1, #d1): small fixnum
 * - #b010 (#x2, #d2): pair
 * - #b011 (#x3, #d3): symbol
 * - #b111 (#x7, #d7): immediate constants ('(), #t, #f, etc.)
 */
#define R_SEXP_TAG_MASK             0x03
#define R_SEXP_BOXED_TAG            0x00
#define R_SEXP_SMALL_FIXNUM_TAG     0x01
#define R_SEXP_PAIR_TAG             0x02
#define R_SEXP_SYMBOL_TAG           0x03
#define R_SEXP_IMMEDIATE_TAG        0x07

#define R_SEXP_MAKE_IMMEDIATE(n)    ((rsexp)((n << 3) | R_SEXP_IMMEDIATE_TAG))
#define R_SEXP_NULL                 R_SEXP_MAKE_IMMEDIATE(0)
#define R_SEXP_FALSE                R_SEXP_MAKE_IMMEDIATE(1)
#define R_SEXP_TRUE                 R_SEXP_MAKE_IMMEDIATE(2)
#define R_SEXP_EOF                  R_SEXP_MAKE_IMMEDIATE(3)
#define R_SEXP_UNSPECIFIED          R_SEXP_MAKE_IMMEDIATE(4)
#define R_SEXP_UNDEFINED            R_SEXP_MAKE_IMMEDIATE(5)

#define r_null_p(sexp)              ((sexp) == R_SEXP_NULL)
#define r_boolean_p(sexp)           ((sexp) == R_SEXP_TRUE || (sexp) == R_SEXP_FALSE)
#define r_eof_p(sexp)               ((sexp) == R_SEXP_EOF)
#define r_unspecified_p(sexp)       ((sexp) == R_SEXP_UNSPECIFIED)
#define r_undefined_p(sexp)         ((sexp) == R_SEXP_UNDEFINED)

#define R_SEXP_NEW(sexp, type)\
        rsexp sexp = (rsexp)GC_NEW(RBoxed);\
        r_boxed_set_type(sexp, type)

#endif  //  __ROSE_SEXP_H__
