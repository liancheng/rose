#ifndef __ROSE_DETAIL_READER_H__
#define __ROSE_DETAIL_READER_H__

#include "detail/lexer.h"
#include "rose/context.h"

#include <setjmp.h>

struct RDatumReader {
    RContext* context;
    rsexp     input_port;
    rsexp     last_error;
    RLexer*   lexer;
    RToken*   lookahead;
    jmp_buf   jmp;
};

#endif  //  __ROSE_DETAIL_READER_H__
