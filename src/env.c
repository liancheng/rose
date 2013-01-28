#include "rose/env.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/primitive.h"
#include "rose/symbol.h"

static rsexp init_primitives (RState* r, rsexp env, RPrimitiveDesc const* desc)
{
    rsexp name;
    rsexp prim;

    for (; desc->name; ++desc) {
        name = r_intern (r, desc->name);
        prim = r_primitive_new (r, desc->name, desc->func, desc->required,
                                desc->optional, desc->rest_p);
        env = r_env_bind_x (r, env, name, prim);
    }

    return env;
}

rsexp env_extend (RState* r, rsexp env, rsexp formals, rsexp vals)
{
    rsexp frame;
    ensure (frame = r_cons (r, formals, vals));
    return r_cons (r, frame, env);
}

rsexp r_env_lookup (RState* r, rsexp env, rsexp var)
{
    if (r_null_p (env))
        return R_UNDEFINED;

    rsexp frame = r_car (env);
    rsexp vars  = r_car (frame);
    rsexp vals  = r_cdr (frame);

    while (!r_null_p (vars)) {
        if (r_eq_p (r, r_car (vars), var))
            return vals;

        vars = r_cdr (vars);
        vals = r_cdr (vals);
    }

    return r_env_lookup (r, r_cdr (env), var);
}

rsexp r_empty_env (RState* r)
{
    return R_NULL;
}

rsexp r_env_bind_x (RState* r, rsexp env, rsexp var, rsexp val)
{
    rsexp vars;
    rsexp vals;
    rsexp frame;

    if (r_null_p (env)) {
        vars = r_list (r, 1, var);
        vals = r_list (r, 1, val);
        return env_extend (r, env, vars, vals);
    }

    ensure (vars = r_cons (r, var, r_car (r_car (env))));
    ensure (vals = r_cons (r, val, r_cdr (r_car (env))));
    ensure (frame = r_cons (r, vars, vals));
    r_set_car_x (env, frame);

    return env;
}

rsexp r_env_assign_x (RState* r, rsexp env, rsexp var, rsexp val)
{
    rsexp vars;
    rsexp vals;
    rsexp frame;

    vals = r_env_lookup (r, env, var);

    if (r_undefined_p (vals)) {
        r_error_code (r, R_ERR_UNBOUND_VAR, var);
        return R_FAILURE;
    }

    ensure (vars = r_cons (r, var, r_car (r_car (env))));
    ensure (vals = r_cons (r, val, r_cdr (r_car (env))));
    ensure (frame = r_cons (r, vars, vals));
    r_set_car_x (env, frame);

    return env;
}

extern const RPrimitiveDesc bytevector_primitives [];
extern const RPrimitiveDesc gc_primitives [];
extern const RPrimitiveDesc math_primitives [];
extern const RPrimitiveDesc number_primitives [];
extern const RPrimitiveDesc pair_primitives [];
extern const RPrimitiveDesc io_primitives [];
extern const RPrimitiveDesc read_primitives [];
extern const RPrimitiveDesc string_primitives [];

rsexp default_env (RState* r)
{
    rsexp env = r_empty_env (r);

    env = init_primitives (r, env, bytevector_primitives);
    env = init_primitives (r, env, gc_primitives);
    env = init_primitives (r, env, io_primitives);
    env = init_primitives (r, env, math_primitives);
    env = init_primitives (r, env, number_primitives);
    env = init_primitives (r, env, pair_primitives);
    env = init_primitives (r, env, read_primitives);
    env = init_primitives (r, env, string_primitives);

    return env;
}
