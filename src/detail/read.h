#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/lexer.h"
#include "rose/sexp.h"
#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

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

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_DETAIL_READER_H__ */
