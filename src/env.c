#include "cell.h"

#include "rose/env.h"
#include "rose/pair.h"

#include <assert.h>

#define SEXP_TO_ENV(obj) R_CELL_VALUE (obj).env

static inline void env_set_parent_x (rsexp env, rsexp parent)
{
    SEXP_TO_ENV (env).parent = parent;
}

static inline rsexp r_env_get_parent (rsexp env)
{
    return SEXP_TO_ENV (env).parent;
}

static inline void env_finalize (rpointer obj, rpointer client_data)
{
    r_hash_table_free (SEXP_TO_ENV ((rsexp) obj).bindings);
}

rboolean r_env_p (rsexp obj)
{
    return r_cell_p (obj) &&
           r_cell_get_type (obj) == SEXP_ENV;
}

rsexp r_env_new ()
{
    R_SEXP_NEW (res, SEXP_ENV);

    SEXP_TO_ENV (res).parent   = R_SEXP_UNDEFINED;
    SEXP_TO_ENV (res).bindings = r_hash_table_new (NULL, NULL);

    GC_REGISTER_FINALIZER ((rpointer) res, env_finalize, NULL, NULL, NULL);

    return res;
}

rsexp r_env_extend (rsexp parent, rsexp vars, rsexp vals)
{
    rsexp env = r_env_new ();

    while (!r_null_p (vars)) {
        r_env_define (env, r_car (vars), r_car (vals));
        vars = r_cdr (vars);
        vals = r_cdr (vals);
    }

    env_set_parent_x (env, parent);
    return env;
}

static rsexp frame_lookup (rsexp frame, rsexp var)
{
    RHashTable* bindings;
    rpointer val;

    bindings = SEXP_TO_ENV (frame).bindings;
    val = r_hash_table_get (bindings, (rconstpointer) var);

    return val ? (rsexp) val : R_SEXP_UNDEFINED;
}

rsexp r_env_lookup (rsexp env, rsexp var)
{
    rsexp frame;
    rsexp val;

    assert (r_env_p (env));

    for (frame = env; !r_undefined_p (frame); ) {
        val = frame_lookup (frame, var);

        if (r_undefined_p (val))
            frame = r_env_get_parent (frame);
    }

    return val;
}

void r_env_define (rsexp env, rsexp var, rsexp val)
{
    assert (r_cell_get_type (env) == SEXP_ENV);
    RHashTable* bindings = SEXP_TO_ENV (env).bindings;
    r_hash_table_put (bindings, (rpointer) var, (rpointer) val);
}

void r_env_set_x (rsexp env, rsexp var, rsexp val)
{
    assert (r_cell_get_type (env) == SEXP_ENV);
    RHashTable* bindings = SEXP_TO_ENV (env).bindings;
    r_hash_table_put (bindings, (rpointer) var, (rpointer) val);
}
