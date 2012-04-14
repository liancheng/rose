#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include "rose/context.h"

#include <glib.h>
#include <stdint.h>

typedef enum {
    SEXP_BOOLEAN,
    SEXP_PAIR,
    SEXP_SYMBOL,
}
r_sexp_types;

typedef struct r_sexp_struct* r_sexp;

typedef uintptr_t r_word;

struct r_sexp_struct {
    r_word tag;

    union {
        struct {
            r_sexp car;
            r_sexp cdr;
        }
        pair;
    }
    value;
};

/*
 * Simple tagging.  Ends in:
 *
 * - 0000 ( 0): pointer to boxed object
 * - 0001 ( 1): symbol
 * - 1110 (14): other immediate constants ('(), #t, #f, etc.)
 */
#define SEXP_BOXED_TAG      0x00
#define SEXP_SYMBOL_TAG     0x01

#define MAKE_IMMEDIATE(n)   ((r_sexp)((n << 4) + 0x0e))
#define SEXP_NULL           MAKE_IMMEDIATE(0)
#define SEXP_FALSE          MAKE_IMMEDIATE(1)
#define SEXP_TRUE           MAKE_IMMEDIATE(2)
#define SEXP_EOF            MAKE_IMMEDIATE(3)
#define SEXP_UNDEFINED      MAKE_IMMEDIATE(4)

#define sexp_null_p(s)      ((s) == SEXP_NULL)
#define sexp_boxed_p(s)     (((r_word)(s) & 0x03) == SEXP_BOXED_TAG)
#define sexp_symbol_p(s)    (((r_word)(s) & 0x07) == SEXP_SYMBOL_TAG)
#define sexp_pair_p(s)      (sexp_boxed_p(s) &&\
                             ((r_sexp)(s))->tag == SEXP_PAIR)

#endif  //  __ROSE_SEXP_H__
