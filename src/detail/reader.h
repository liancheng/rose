#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/lexer.h"
#include "rose/context.h"

#include <setjmp.h>

struct RReaderState {
    RContext* context;
    rsexp     input_port;
    rsexp     tree;
    rsexp     error_type;
    rsexp     last_error;
    RLexer*   lexer;
};

#endif  //  __ROSE_DETAIL_READER_H__
