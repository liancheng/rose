#ifndef __ROSE_PARSER_H__
#define __ROSE_PARSER_H__

#include "rose/sexp.h"
#include "scanner.h"

#include <setjmp.h>

typedef struct RReaderState RReaderState;

struct RReaderState {
    rsexp     context;
    rsexp     input_port;
    rsexp     tree;
    rsexp     error_type;
    rsexp     last_error;
    RScanner* scanner;
    jmp_buf   jmp;
};

RReaderState* r_reader_new (rsexp context);

#endif  //  __ROSE_PARSER_H__
