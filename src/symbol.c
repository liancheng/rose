#include "rose/symbol.h"

#include <glib.h>

rquark r_quark_from_symbol(char const* symbol, RContext* context)
{
    return g_quark_from_string(symbol);
}

rquark r_quark_from_static_symbol(char const* symbol, RContext* context)
{
    return g_quark_from_static_string(symbol);
}

char const* r_quark_to_symbol(rquark quark, RContext* context)
{
    return g_quark_to_string(quark);
}

rsexp r_sexp_from_symbol(char const* symbol, RContext* context)
{
    rquark quark = r_quark_from_symbol(symbol, context);
    return (rsexp)((quark << 3) | SEXP_SYMBOL_TAG);
}

rsexp r_sexp_from_static_symbol(char const* symbol, RContext* context)
{
    rquark quark = r_quark_from_static_symbol(symbol, context);
    return (rsexp)((quark << 3) | SEXP_SYMBOL_TAG);
}

char const* r_sexp_to_symbol(rsexp sexp, RContext* context)
{
    g_assert(SEXP_SYMBOL_P(sexp));
    return r_quark_to_symbol((sexp) >> 3, context);
}
