#include "rose/env.h"
#include "rose/hash.h"
#include "rose/pair.h"

#include <assert.h>
#include <gc/gc.h>

static inline void sexp_env_set_parent(rsexp env, rsexp parent)
{
    SEXP_AS(env, env).parent = parent;
}

static inline rsexp sexp_env_get_parent(rsexp env)
{
    return SEXP_AS(env, env).parent;
}

rsexp sexp_env_new()
{
    rsexp res = (rsexp)GC_NEW(RBoxed);

    SEXP_TYPE(res)             = SEXP_ENV;
    SEXP_AS(res, env).parent   = SEXP_NULL;
    SEXP_AS(res, env).bindings = r_hash_table_new(NULL, NULL);

    return res;
}

rsexp sexp_env_extend(rsexp parent, rsexp vars, rsexp vals)
{
    rsexp env = sexp_env_new();

    while (!SEXP_NULL_P(vars)) {
        sexp_env_define(env, sexp_car(vars), sexp_car(vals));
        vars = sexp_cdr(vars);
        vals = sexp_cdr(vals);
    }

    sexp_env_set_parent(env, parent);
    return env;
}

rsexp sexp_env_lookup(rsexp env, rsexp var)
{
    assert(SEXP_CHECK_TYPE(env, SEXP_ENV));
    RHashTable* bindings = SEXP_AS(env, env).bindings;
    return (rsexp)r_hash_table_get(bindings, (rconstpointer)var);
}

void sexp_env_define(rsexp env, rsexp var, rsexp val)
{
    assert(SEXP_CHECK_TYPE(env, SEXP_ENV));
    RHashTable* bindings = SEXP_AS(env, env).bindings;
    r_hash_table_put(bindings, (rpointer)var, (rpointer)val);
}

void sexp_env_set(rsexp env, rsexp var, rsexp val)
{
    assert(SEXP_CHECK_TYPE(env, SEXP_ENV));
    RHashTable* bindings = SEXP_AS(env, env).bindings;
    r_hash_table_put(bindings, (rpointer)var, (rpointer)val);
}
