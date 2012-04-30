#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    SEXP_STRING,
    SEXP_VECTOR,
    SEXP_INPUT_PORT,
    SEXP_OUTPUT_PORT,
    SEXP_ERROR,
}
r_boxed_types;

typedef uintptr_t r_word;
typedef uintptr_t r_sexp;

typedef struct r_pair {
    r_sexp car;
    r_sexp cdr;
}
r_pair;

typedef struct r_string {
    size_t length;
    char* data;
}
r_string;

typedef struct r_vector {
    size_t size;
    r_sexp* data;
}
r_vector;

typedef struct r_input_port {
    FILE* stream;
}
r_input_port;

typedef struct r_output_port {
    FILE* stream;
}
r_output_port;

typedef struct r_error {
    r_sexp message;
    r_sexp irritants;
}
r_error;

typedef struct r_boxed {
    int type;

    union {
        r_string string;
        r_vector vector;
        r_input_port input_port;
        r_output_port output_port;
        r_error error;
    }
    as;
}
r_boxed;

/*
 * Simple tagging.  Ends in:
 *
 * - #b000 (#x0, #d0): pointer to boxed object
 * - #b001 (#x1, #d1): small fixnum
 * - #b010 (#x2, #d2): pair
 * - #b011 (#x3, #d3): symbol
 * - #b111 (#x7, #d7): immediate constants ('(), #t, #f, etc.)
 */
#define SEXP_BOXED_TAG          0x00
#define SEXP_SMALL_FIXNUM_TAG   0x01
#define SEXP_PAIR_TAG           0x02
#define SEXP_SYMBOL_TAG         0x03
#define SEXP_IMMEDIATE_TAG      0x07

#define SEXP_MAKE_IMMEDIATE(n)  ((r_sexp)((n << 3) | SEXP_IMMEDIATE_TAG))
#define SEXP_NULL               SEXP_MAKE_IMMEDIATE(0)
#define SEXP_FALSE              SEXP_MAKE_IMMEDIATE(1)
#define SEXP_TRUE               SEXP_MAKE_IMMEDIATE(2)
#define SEXP_EOF                SEXP_MAKE_IMMEDIATE(3)
#define SEXP_UNSPECIFIED        SEXP_MAKE_IMMEDIATE(4)
#define SEXP_UNDEFINED          SEXP_MAKE_IMMEDIATE(5)

#define SEXP_MAKE_SMALL_FIXNUM(n)   ((n << 3) | SEXP_SMALL_FIXNUM_TAG)
#define SEXP_ZERO                   SEXP_MAKE_S_FIXNUM(1)
#define SEXP_ONE                    SEXP_MAKE_S_FIXNUM(1)

#define SEXP_NULL_P(s)          ((s) == SEXP_NULL)
#define SEXP_BOXED_P(s)         (((s) & 0x03) == SEXP_BOXED_TAG)
#define SEXP_BOOLEAN_P(s)       ((s) == SEXP_TRUE || (s) == SEXP_FALSE)
#define SEXP_EOF_P(s)           ((s) == SEXP_EOF)
#define SEXP_UNSPECIFIED_P(s)   ((s) == SEXP_UNSPECIFIED)
#define SEXP_UNDEFINED_P(s)     ((s) == SEXP_UNDEFINED)
#define SEXP_TYPE(s)            (((r_boxed*)s)->type)
#define SEXP_CHECK_TYPE(s, t)   (SEXP_BOXED_P(s) && SEXP_TYPE(s) == t)

#endif  //  __ROSE_SEXP_H__
