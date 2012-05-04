#include "boxed.h"
#include "keyword.h"
#include "opaque.h"
#include "scanner.h"

#include "rose/context.h"
#include "rose/env.h"
#include "rose/symbol.h"

rsexp r_context_new()
{
    rsexp context      = r_vector_new(CTX_N_FIELD);
    rsexp scanner      = r_opaque_new(r_scanner_new());
    rsexp symbol_table = r_opaque_new(r_symbol_table_new());
    rsexp env          = r_env_new();
    rsexp keywords     = r_keywords_init(context);

    r_vector_set_x(context, CTX_SCANNER,      scanner);
    r_vector_set_x(context, CTX_SYMBOL_TABLE, symbol_table);
    r_vector_set_x(context, CTX_KEYWORDS,     keywords);
    r_vector_set_x(context, CTX_ENV,          env);

    return context;
}

rsexp r_context_field(rsexp context, rint name)
{
    assert(name > 0 && name < CTX_N_FIELD);
    return r_vector_ref(context, name);
}
