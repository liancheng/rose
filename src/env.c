#include "detail/env.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/pair.h"

static void unbound_variable (RState* state, rsexp var)
{
    r_error_format (state, "unbound variable: ~s", var);
}

rsexp lookup (RState* state, rsexp env, rsexp var)
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

    return lookup (state, r_cdr (env), var);
}

rsexp extend (RState* state, rsexp env, rsexp vars, rsexp vals)
{
    return r_cons (state, r_cons (state, vars, vals), env);
}

rsexp empty_env (RState* state)
{
    return R_NULL;
}

rsexp bind_x (RState* state, rsexp env, rsexp var, rsexp val)
{
    rsexp vars;
    rsexp vals;
    rsexp frame;

    if (r_null_p (env)) {
        vars = r_list (state, 1, var);
        vals = r_list (state, 1, val);
        return extend (state, env, vars, vals);
    }

    vals = lookup (state, env, var);

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

rsexp assign_x (RState* state, rsexp env, rsexp var, rsexp val)
{
    rsexp vars;
    rsexp vals;
    rsexp frame;

    vals = lookup (state, env, var);

    if (r_undefined_p (vals)) {
        unbound_variable (state, var);
        return R_FAILURE;
    }

    vars = r_cons (state, var, r_car (r_car (env)));
    vals = r_cons (state, val, r_cdr (r_car (env)));
    frame = r_cons (state, vars, vals);
    r_set_car_x (env, frame);

    return env;
}
