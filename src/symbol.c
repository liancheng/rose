#include "detail/state.h"
#include "detail/symbol.h"
#include "rose/io.h"

#include <glib.h>

static rsexp write_symbol (RState* r, rsexp port, rsexp obj)
{
    return r_port_puts (r, port, r_symbol_name (r, obj));
}

rsexp r_symbol_new (RState* r, rconstcstring symbol)
{
    return quark_to_sexp (g_quark_from_string (symbol));
}

rsexp r_symbol_new_static (RState* r, rconstcstring symbol)
{
    return quark_to_sexp (g_quark_from_static_string (symbol));
}

rconstcstring r_symbol_name (RState* r, rsexp obj)
{
    return g_quark_to_string (quark_from_sexp (obj));
}

RTypeInfo symbol_type = {
    .size = 0,
    .name = "symbol",
    .ops = {
        .write = write_symbol,
        .display = write_symbol
    }
};
