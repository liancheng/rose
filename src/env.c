#include "detail/hash.h"
#include "detail/sexp.h"
#include "rose/env.h"
#include "rose/pair.h"
#include "rose/port.h"

#include <assert.h>
#include <gc/gc.h>

struct REnv{
    RType*      type;
    rsexp       parent;
    RHashTable* bindings;
};

#define ENV_FROM_SEXP(obj)  (*((REnv*) obj))
#define ENV_TO_SEXP(env)    ((rsexp) env)

static RType* r_env_type_info ();

static inline void env_set_parent_x (rsexp env, rsexp parent)
{
    ENV_FROM_SEXP (env).parent = parent;
}

static inline rsexp r_env_get_parent (rsexp env)
{
    return ENV_FROM_SEXP (env).parent;
}

static inline void env_finalize (rpointer obj, rpointer client_data)
{
    r_hash_table_free (ENV_FROM_SEXP ((rsexp) obj).bindings);
}

static void r_env_write (rsexp port, rsexp obj)
{
    r_port_printf (port, "#<%s>", r_env_type_info ()->name);
}

static RType* r_env_type_info ()
{
    static RType* type = NULL;

    if (!type) {
        type = GC_NEW (RType);

        type->cell_size  = sizeof (REnv);
        type->name       = "environment";
        type->write_fn   = r_env_write;
        type->display_fn = r_env_write;
    }

    return type;
}

rboolean r_env_p (rsexp obj)
{
    return r_cell_p (obj) &&
           R_CELL_TYPE (obj) == r_env_type_info ();
}

rsexp r_env_new ()
{
    REnv* res = GC_NEW (REnv);

    res->type     = r_env_type_info ();
    res->parent   = R_UNDEFINED;
    res->bindings = r_hash_table_new (NULL, NULL);

    GC_REGISTER_FINALIZER ((rpointer) res, env_finalize, NULL, NULL, NULL);

    return ENV_TO_SEXP (res);
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

    bindings = ENV_FROM_SEXP (frame).bindings;
    val = r_hash_table_get (bindings, (rconstpointer) var);

    return val ? (rsexp) val : R_UNDEFINED;
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
    assert (r_env_p (env));
    RHashTable* bindings = ENV_FROM_SEXP (env).bindings;
    r_hash_table_put (bindings, (rpointer) var, (rpointer) val);
}

void r_env_set_x (rsexp env, rsexp var, rsexp val)
{
    assert (r_env_p (env));
    RHashTable* bindings = ENV_FROM_SEXP (env).bindings;
    r_hash_table_put (bindings, (rpointer) var, (rpointer) val);
}
