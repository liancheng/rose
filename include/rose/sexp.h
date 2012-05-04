#ifndef __ROSE_SEXP_H__
#define __ROSE_SEXP_H__

#include "rose/hash.h"

#include <stdio.h>

typedef enum {
    SEXP_ENV,
    SEXP_STRING,
    SEXP_VECTOR,
    SEXP_INPUT_PORT,
    SEXP_OUTPUT_PORT,
    SEXP_ERROR,
}
RBoxedTypes;

typedef rword rsexp;

typedef struct RPair {
    rsexp car;
    rsexp cdr;
}
RPair;

typedef struct REnv {
    rsexp       parent;
    RHashTable* bindings;
}
REnv;

typedef struct RString {
    rsize length;
    char* data;
}
RString;

typedef struct RVector {
    rsize  size;
    rsexp* data;
}
RVector;

typedef struct RInputPort {
    FILE* stream;
}
RInputPort;

typedef struct ROutputPort {
    FILE* stream;
}
ROutputPort;

typedef struct RError {
    rsexp message;
    rsexp irritants;
}
RError;

typedef struct RBoxed {
    int type;

    union {
        REnv        env;
        RString     string;
        RVector     vector;
        RInputPort  input_port;
        ROutputPort output_port;
        RError      error;
    }
    value;
}
RBoxed;

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

#define SEXP_MAKE_IMMEDIATE(n)  ((rsexp)((n << 3) | SEXP_IMMEDIATE_TAG))
#define SEXP_NULL               SEXP_MAKE_IMMEDIATE(0)
#define SEXP_FALSE              SEXP_MAKE_IMMEDIATE(1)
#define SEXP_TRUE               SEXP_MAKE_IMMEDIATE(2)
#define SEXP_EOF                SEXP_MAKE_IMMEDIATE(3)
#define SEXP_UNSPECIFIED        SEXP_MAKE_IMMEDIATE(4)
#define SEXP_UNDEFINED          SEXP_MAKE_IMMEDIATE(5)

#define SEXP_MAKE_S_FIXNUM(n)   ((n << 3) | SEXP_SMALL_FIXNUM_TAG)
#define SEXP_ZERO               SEXP_MAKE_S_FIXNUM(1)
#define SEXP_ONE                SEXP_MAKE_S_FIXNUM(1)

#define SEXP_NULL_P(s)          ((s) == SEXP_NULL)
#define SEXP_BOXED_P(s)         (((s) & 0x03) == SEXP_BOXED_TAG)
#define SEXP_BOOLEAN_P(s)       ((s) == SEXP_TRUE || (s) == SEXP_FALSE)
#define SEXP_EOF_P(s)           ((s) == SEXP_EOF)
#define SEXP_UNSPECIFIED_P(s)   ((s) == SEXP_UNSPECIFIED)
#define SEXP_UNDEFINED_P(s)     ((s) == SEXP_UNDEFINED)
#define SEXP_TYPE(s)            (((RBoxed*)s)->type)
#define SEXP_CHECK_TYPE(s, t)   (SEXP_BOXED_P(s) && SEXP_TYPE(s) == t)
#define SEXP_AS(s, t)           (((RBoxed*)s)->value.t)

#endif  //  __ROSE_SEXP_H__
