#include "detail/state.h"
#include "rose/gc.h"
#include "rose/port.h"
#include "rose/symbol.h"

#include <glib.h>

#define quark_to_sexp(q)    (((q) << R_TAG_BITS) | R_SYMBOL_TAG)
#define quark_from_sexp(q)  (r_cast (GQuark, ((q) >> R_TAG_BITS)))

static rsexp write_symbol (RState* state, rsexp port, rsexp obj)
{
    return r_port_puts (state, port, r_symbol_name (state, obj));
}

rsexp r_symbol_new (RState* state, rconstcstring symbol)
{
    return quark_to_sexp (g_quark_from_string (symbol));
}

rsexp r_symbol_new_static (RState* state, rconstcstring symbol)
{
    return quark_to_sexp (g_quark_from_static_string (symbol));
}

rconstcstring r_symbol_name (RState* state, rsexp obj)
{
    return g_quark_to_string (quark_from_sexp (obj));
}

void init_symbol_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = 0;
    type->name         = "symbol";
    type->ops.write    = write_symbol;
    type->ops.display  = write_symbol;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.mark     = NULL;
    type->ops.finalize = NULL;

    state->builtin_types [R_SYMBOL_TAG] = type;
}
