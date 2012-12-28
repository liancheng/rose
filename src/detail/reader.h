#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/lexer.h"
#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

typedef struct RDatumReader RDatumReader;

struct RDatumReader {
    RState* state;
    rsexp   input_port;
    rsexp   last_error;
    RLexer  lexer;
    RToken* lookahead;
    RToken  token;
    rsize   current_line;
    rsize   current_column;
};

R_END_DECLS

#endif  /* __ROSE_DETAIL_READER_H__ */
