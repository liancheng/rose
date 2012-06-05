#ifndef __ROSE_DETAIL_CONTEXT_H__
#define __ROSE_DETAIL_CONTEXT_H__

#include "rose/context.h"
#include "rose/sexp.h"
#include "rose/symbol.h"

struct RContext {
    RSymbolTable* symbol_table;
    rsexp env;
    rsexp current_input_port;
    rsexp current_output_port;
};

#endif  //  __ROSE_DETAIL_CONTEXT_H__
