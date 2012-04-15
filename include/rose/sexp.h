#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include "rose/context.h"

#include <stdint.h>

typedef enum {
    SEXP_BOOLEAN,
    SEXP_PAIR,
    SEXP_SYMBOL,
}
r_sexp_types;

typedef uintptr_t r_sexp;

typedef struct _r_pair {
    r_sexp car;
    r_sexp cdr;
}
r_pair;

/*
 * Simple tagging.  Ends in:
 *
 * - #b0000 (#x0, #d00): pointer to boxed object
 * - #b0001 (#x1, #d01): symbol
 * - #b0011 (#x3, #d03): pair
 * - #b1110 (#xe, #d14): immediate constants ('(), #t, #f, etc.)
 */
#define SEXP_BOXED_TAG      0x00
#define SEXP_SYMBOL_TAG     0x01
#define SEXP_PAIR_TAG       0x03

#define SEXP_FAIL           (0)

#define SEXP_MAKE_CONST(n)  ((r_sexp)((n << 4) + 0x0e))
#define SEXP_NULL           SEXP_MAKE_CONST(0)
#define SEXP_FALSE          SEXP_MAKE_CONST(1)
#define SEXP_TRUE           SEXP_MAKE_CONST(2)
#define SEXP_EOF            SEXP_MAKE_CONST(3)
#define SEXP_SPECIFIED      SEXP_MAKE_CONST(4)
#define SEXP_UNDEFINED      SEXP_MAKE_CONST(5)

#define SEXP_NULL_P(s)      ((s) == SEXP_NULL)
#define SEXP_BOXED_P(s)     (((s) & 0x03) == SEXP_BOXED_TAG)
#define SEXP_BOOLEAN_P(s)   ((s == SEXP_TRUE) || (s == SEXP_FALSE))

#endif  //  __ROSE_SEXP_H__
