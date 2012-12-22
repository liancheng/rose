#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/lexer.h"
#include "rose/sexp.h"
#include "rose/state.h"

#include <setjmp.h>

typedef struct RDatumReader RDatumReader;

struct RDatumReader {
    RState* state;
    jmp_buf jmp;
    rsexp   input_port;
    rsexp   last_error;
    RLexer  lexer;
    RToken* lookahead;
    RToken  token;
    rsize   current_line;
    rsize   current_column;
};

#endif  /* __ROSE_DETAIL_READER_H__ */
