#include "detail/state.h"
#include "rose/gc.h"
#include "rose/port.h"
#include "rose/symbol.h"

#include <glib.h>

#define quark_to_sexp(q)    (r_cast (rsexp, r_set_tag_x ((q), R_TAG_SYMBOL)))
#define quark_from_sexp(q)  (r_cast (GQuark, ((q) >> R_TAG_BITS)))

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
        .write   = write_symbol,
        .display = write_symbol
    }
};
