#include "rose/context.h"
#include "rose/env.h"
#include "rose/memory.h"
#include "rose/scanner.h"
#include "rose/symbol.h"

struct RContext {
    RScanner*     scanner;
    RSymbolTable* symbol_table;
};

#define DEFINE_CONTEXT_GET_FIELD(field)\
        rpointer r_context_get_##field(RContext* context)\
        {\
            return (rpointer)context->field;\
        }

DEFINE_CONTEXT_GET_FIELD(scanner);
DEFINE_CONTEXT_GET_FIELD(symbol_table);

RContext* r_context_new()
{
    RContext* context = R_NEW(RContext, 1);

    context->scanner      = r_scanner_new();
    context->symbol_table = r_symbol_table_new();

    return context;
}

void r_context_free(RContext* context)
{
    r_scanner_free(context->scanner);
    free(context);
}
