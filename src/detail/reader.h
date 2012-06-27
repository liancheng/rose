#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/lexer.h"
#include "rose/context.h"

struct RDatumReader {
    RContext* context;
    rsexp     input_port;
    rsexp     tree;
    rsexp     error_type;
    rsexp     last_error;
    RLexer*   lexer;
    RToken*   lookahead;
};

#endif  //  __ROSE_DETAIL_READER_H__
