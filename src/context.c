#include "rose/context.h"
#include "rose/env.h"
#include "rose/memory.h"
#include "rose/scanner.h"
#include "rose/symbol.h"

struct RContext {
    RScanner*     scanner;
    RSymbolTable* symbol_table;
    rsexp         keywords;
    rsexp         global_env;
};

#define DEFINE_CONTEXT_FIELD_GETTER(field)\
        rpointer r_context_get_##field(RContext* context)\
        {\
            return (rpointer)context->field;\
        }

DEFINE_CONTEXT_FIELD_GETTER(scanner);
DEFINE_CONTEXT_FIELD_GETTER(symbol_table);
DEFINE_CONTEXT_FIELD_GETTER(keywords);
DEFINE_CONTEXT_FIELD_GETTER(global_env);

RContext* r_context_new()
{
    RContext* context = R_NEW(RContext, 1);

    context->scanner      = r_scanner_new();
    context->symbol_table = r_symbol_table_new();
    context->global_env   = sexp_env_new();
    context->keywords     = sexp_keywords(context);

    return context;
}

void r_context_free(RContext* context)
{
    r_scanner_free(context->scanner);
    free(context);
}
