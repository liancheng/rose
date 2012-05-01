#include "rose/context.h"
#include "rose/memory.h"
#include "rose/scanner.h"

#include <glib.h>

struct RContext {
    RScanner*     scanner;
};

RContext* r_context_new()
{
    RContext* context = R_NEW(RContext, 1);

    context->scanner = r_scanner_new();

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

