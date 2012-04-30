#include "rose/scanner.h"
#include "rose/context.h"

#include <glib.h>

RContext* r_context_new()
{
    RContext* res = malloc(sizeof(RContext));
    res->scanner = r_scanner_new();
    return res;
}

void r_context_free(RContext* context)
{
    r_scanner_free(context->scanner);
    free(context);
}
