#include "rose/symbol.h"

#include <glib.h>

r_quark quark_from_symbol(char const* symbol, r_context* context)
{
    return g_quark_from_string(symbol);
}

r_quark quark_from_static_symbol(char const* symbol, r_context* context)
{
    return g_quark_from_static_string(symbol);
}

char const* quark_to_symbol(r_quark quark, r_context* context)
{
    return g_quark_to_string(quark);
}

r_sexp sexp_from_symbol(char const* symbol, r_context* context)
{
    r_quark quark = quark_from_symbol(symbol, context);
    return (r_sexp)((quark << 2) + SEXP_SYMBOL_TAG);
}

r_sexp sexp_from_static_symbol(char const* symbol, r_context* context)
{
    r_quark quark = quark_from_static_symbol(symbol, context);
    return (r_sexp)((quark << 2) + SEXP_SYMBOL_TAG);
}

char const* sexp_to_symbol(r_sexp sexp, r_context* context)
{
    g_assert(SEXP_SYMBOL_P(sexp));
    return quark_to_symbol((sexp) >> 2, context);
}