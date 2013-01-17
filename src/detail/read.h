#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/lexer.h"
#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

typedef struct Reader Reader;

struct Reader {
    RState* r;
    rsexp   input_port;
    RLexer  lexer;
    RToken* lookahead;
    RToken  token;
    rsize   current_line;
    rsize   current_column;
};

R_END_DECLS

#endif /* __ROSE_DETAIL_READER_H__ */
