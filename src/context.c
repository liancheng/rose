#include "rose/context.h"
#include "rose/env.h"
#include "rose/memory.h"
#include "rose/scanner.h"

#include <glib.h>

struct RContext {
    RScanner* scanner;
    rsexp     global_env;
};

RContext* r_context_new()
{
    RContext* context = R_NEW(RContext, 1);

    context->scanner = r_scanner_new();
    context->global_env = sexp_env_new();

    return context;
}

void r_context_free(RContext* context)
{
    r_scanner_free(context->scanner);
    free(context);
}

RScanner* r_context_get_scanner(RContext* context)
{
    return context->scanner;
}

rsexp r_context_get_global_env(RContext* context)
{
    return context->global_env;
}
