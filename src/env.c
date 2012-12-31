#include "detail/env.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/primitive.h"
#include "rose/symbol.h"

rsexp env_extend (RState* state, rsexp env, rsexp vars, rsexp vals)
{
    return r_cons (state, r_cons (state, vars, vals), env);
}

rsexp r_env_lookup (RState* state, rsexp env, rsexp var)
{
    if (r_null_p (env))
        return R_UNDEFINED;

    rsexp frame = r_car (env);
    rsexp vars  = r_car (frame);
    rsexp vals  = r_cdr (frame);

    while (!r_null_p (vars)) {
        if (r_eq_p (state, r_car (vars), var))
            return vals;

        vars = r_cdr (vars);
        vals = r_cdr (vals);
    }

    return r_env_lookup (state, r_cdr (env), var);
}

rsexp r_empty_env (RState* state)
{
    return R_NULL;
}

rsexp r_env_bind_x (RState* state, rsexp env, rsexp var, rsexp val)
{
    rsexp vars;
    rsexp vals;
    rsexp frame;

    if (r_null_p (env)) {
        vars = r_list (state, 1, var);
        vals = r_list (state, 1, val);
        return env_extend (state, env, vars, vals);
    }

    vals = r_env_lookup (state, env, var);

    if (!r_undefined_p (vals)) {
        r_set_car_x (vals, val);
        return env;
    }

    vars = r_cons (state, var, r_car (r_car (env)));
    vals = r_cons (state, val, r_cdr (r_car (env)));
    frame = r_cons (state, vars, vals);
    r_set_car_x (env, frame);

    return env;
}

rsexp r_env_assign_x (RState* state, rsexp env, rsexp var, rsexp val)
{
    rsexp vars;
    rsexp vals;
    rsexp frame;

    vals = r_env_lookup (state, env, var);

    if (r_undefined_p (vals)) {
        r_error_code (state, R_ERR_UNBOUND_VAR, var);
        return R_FAILURE;
    }

    vars = r_cons (state, var, r_car (r_car (env)));
    vals = r_cons (state, val, r_cdr (r_car (env)));
    frame = r_cons (state, vars, vals);
    r_set_car_x (env, frame);

    return env;
}

void bind_primitive_x (RState* state,
                       rsexp* env,
                       rconstcstring name,
                       RPrimitiveFunc func,
                       rsize required,
                       rsize optional,
                       rbool rest_p)
{
    rsexp name_id = r_symbol_new (state, name);
    *env = r_env_bind_x (state, *env, name_id,
            r_primitive_new (state, name, func, required, optional, rest_p));
}

void pair_init_primitives (RState* state, rsexp* env);
void port_init_primitives (RState* state, rsexp* env);

rsexp default_env (RState* state)
{
    rsexp env = r_empty_env (state);

    pair_init_primitives (state, &env);
    port_init_primitives (state, &env);

    return env;
}
