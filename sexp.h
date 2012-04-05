#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include <stdint.h>

enum sexp_types {
    SEXP_BOOLEAN,
    SEXP_PAIR,
    SEXP_SYMBOL,
};

typedef struct sexp_struct* sexp;

typedef void* sexp_word;

struct sexp_struct {
    sexp_word tag;

    union {
        struct {
            sexp car;
            sexp cdr;
        }
        pair;

        struct {
            sexp_word length;
            char name[];
        }
        symbol;
    }
    value;
};

/**
 * Simple tagging.  Ends in:
 *
 * - 0000 ( 0): pointer
 * - 0011 ( 3): long symbol
 * - 0111 ( 7): immediate symbol
 * - 1110 (14): other immediate constants ('(), #t, #f, etc.)
 */
#define SEXP_POINTER_TAG    0x00
#define SEXP_L_SYMBOL_TAG   0x03
#define SEXP_I_SYMBOL_TAG   0x07

#define MAKE_IMMEDIATE(n)   ((sexp)((n << 4) + 0x0e))
#define SEXP_NULL           MAKE_IMMEDIATE(0)
#define SEXP_FALSE          MAKE_IMMEDIATE(1)
#define SEXP_TRUE           MAKE_IMMEDIATE(2)
#define SEXP_EOF            MAKE_IMMEDIATE(3)
#define SEXP_UNDEFINED      MAKE_IMMEDIATE(4)

#define SEXP_NULL_P(s)      ((s) == SEXP_NULL)
#define SEXP_POINTER_P(s)   (((sexp_word)(s) & 0x03) == SEXP_POINTER_TAG)
#define SEXP_I_SYMBOL_P(s)  (((sexp_word)(s) & 0x07) == SEXP_I_SYMBOL_TAG)
#define SEXP_L_SYMBOL_P(s)  (((sexp_word)(s) & 0x07) == SEXP_L_SYMBOL_TAG)
#define SEXP_SYMBOL_P(s)    (SEXP_I_SYMBOL_P(s) || SEXP_L_SYMBOL_P(s))
#define SEXP_PAIR_P(s)      (SEXP_POINTER_P(s) && ((sexp)(s))->tag == SEXP_PAIR)

#endif  //  __ROSE_SEXP_H__
