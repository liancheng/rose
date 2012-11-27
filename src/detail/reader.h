#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/lexer.h"
#include "rose/sexp.h"
#include "rose/state.h"

#include <setjmp.h>

struct RDatumReader {
    jmp_buf jmp;
    rsexp   input_port;
    rsexp   last_error;
    RLexer* lexer;
    RToken* lookahead;
    RToken  token;
};

#endif  /* __ROSE_DETAIL_READER_H__ */
