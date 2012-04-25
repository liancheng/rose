#include "rose/scanner.h"
#include "rose/context.h"

#include <glib.h>

r_context* context_new()
{
    r_context* res = malloc(sizeof(r_context));
    res->scanner = scanner_new();
    return res;
}

void context_free(r_context* context)
{
    scanner_free(context->scanner);
    free(context);
}
