#include "boxed.h"
#include "keyword.h"
#include "opaque.h"
#include "scanner.h"

#include "rose/context.h"
#include "rose/env.h"
#include "rose/symbol.h"

rsexp r_context_new()
{
    rsexp context = r_vector_new(CTX_N_FIELD);

    r_vector_set(context,
                 CTX_SCANNER,
                 r_opaque_new(r_scanner_new()));

    r_vector_set(context,
                 CTX_SYMBOL_TABLE,
                 r_opaque_new(r_symbol_table_new()));

    r_vector_set(context,
                 CTX_KEYWORDS,
                 r_keywords_init(context));

    r_vector_set(context,
                 CTX_ENV,
                 r_env_new());

    r_vector_set(context,
                 CTX_CURRENT_INPUT_PORT,
                 R_SEXP_NULL);

    r_vector_set(context,
                 CTX_CURRENT_OUTPUT_PORT,
                 R_SEXP_NULL);

    return context;
}

rsexp r_context_get(rsexp context, ruint key)
{
    assert(key < CTX_N_FIELD);
    return r_vector_ref(context, key);
}

rsexp r_context_set(rsexp context, ruint key, rsexp value)
{
    assert(key < CTX_N_FIELD);
    return r_vector_set(context, key, value);
}
