#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/context.h"
#include "detail/lexer.h"

#include <setjmp.h>

struct RReaderState {
    RContext*   context;
    rsexp       input_port;
    rsexp       tree;
    rsexp       error_type;
    rsexp       last_error;
    RLexer*     lexer;
    jmp_buf     jmp;
};

#endif  //  __ROSE_DETAIL_READER_H__
