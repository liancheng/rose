#include "rose/env.h"
#include "rose/hash.h"
#include "rose/pair.h"

#include <assert.h>
#include <gc/gc.h>

static inline void r_env_set_parent(rsexp env, rsexp parent)
{
    SEXP_TO_ENV(env).parent = parent;
}

static inline rsexp r_env_get_parent(rsexp env)
{
    return SEXP_TO_ENV(env).parent;
}

rsexp r_env_new()
{
    rsexp res = (rsexp)GC_NEW(RBoxed);

    SEXP_TYPE(res)            = SEXP_ENV;
    SEXP_TO_ENV(res).parent   = SEXP_NULL;
    SEXP_TO_ENV(res).bindings = r_hash_table_new(NULL, NULL);

    return res;
}

rsexp r_env_extend(rsexp parent, rsexp vars, rsexp vals)
{
    rsexp env = r_env_new();

    while (!SEXP_NULL_P(vars)) {
        r_env_define(env, r_car(vars), r_car(vals));
        vars = r_cdr(vars);
        vals = r_cdr(vals);
    }

    r_env_set_parent(env, parent);
    return env;
}

rsexp r_env_lookup(rsexp env, rsexp var)
{
    assert(SEXP_CHECK_TYPE(env, SEXP_ENV));
    RHashTable* bindings = SEXP_TO_ENV(env).bindings;
    return (rsexp)r_hash_table_get(bindings, (rconstpointer)var);
}

void r_env_define(rsexp env, rsexp var, rsexp val)
{
    assert(SEXP_CHECK_TYPE(env, SEXP_ENV));
    RHashTable* bindings = SEXP_TO_ENV(env).bindings;
    r_hash_table_put(bindings, (rpointer)var, (rpointer)val);
}

void r_env_set(rsexp env, rsexp var, rsexp val)
{
    assert(SEXP_CHECK_TYPE(env, SEXP_ENV));
    RHashTable* bindings = SEXP_TO_ENV(env).bindings;
    r_hash_table_put(bindings, (rpointer)var, (rpointer)val);
}
