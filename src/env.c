#include "rose/env.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/primitive.h"
#include "rose/symbol.h"

#include <assert.h>

static rsexp init_primitives (RState* r, rsexp env, RPrimitiveDesc const* desc)
{
    rsexp name;
    rsexp prim;

    for (; desc->name; ++desc) {
        ensure (name = r_intern (r, desc->name));
        ensure (prim = r_primitive_new (r, desc->name, desc->func,
                                        desc->required, desc->optional,
                                        desc->rest_p));
        ensure (env = r_env_bind_x (r, env, name, prim));
    }

    return env;
}

static inline rsexp enclosing_env (rsexp env)
{
    return r_null_p (env) ? R_NULL : r_cdr (env);
}

static inline rsexp first_frame (rsexp env)
{
    return r_null_p (env) ? R_NULL : r_car (env);
}

static inline rsexp make_frame (RState* r, rsexp vars, rsexp vals)
{
    return r_cons (r, vars, vals);
}

static inline rsexp frame_vars (rsexp frame)
{
    return r_null_p (frame) ? R_NULL : r_car (frame);
}

static inline rsexp frame_vals (rsexp frame)
{
    return r_null_p (frame) ? R_NULL : r_cdr (frame);
}

static inline rsexp frame_bind_x (RState* r, rsexp frame, rsexp var, rsexp val)
{
    rsexp vars, vals;

    ensure (vars = r_cons (r, var, frame_vars (frame)));
    ensure (vals = r_cons (r, val, frame_vals (frame)));

    r_set_car_x (frame, vars);
    r_set_cdr_x (frame, vals);

    return frame;
}

rsexp env_extend (RState* r, rsexp env, rsexp vars, rsexp vals)
{
    rsexp frame;
    rsexp vars_len;
    rsexp vals_len;

    assert (!r_failure_p (vars_len = r_length (r, vars)));
    assert (!r_failure_p (vals_len = r_length (r, vals)));

    if (vars_len != vals_len) {
        r_error_code (r, R_ERR_WRONG_ARG_NUM);
        return R_FAILURE;
    }

    ensure (frame = r_cons (r, vars, vals));

    return r_cons (r, frame, env);
}

rsexp r_env_lookup (RState* r, rsexp env, rsexp var)
{
    if (r_null_p (env))
        return R_UNDEFINED;

    rsexp frame = first_frame (env);
    rsexp vars = frame_vars (frame);
    rsexp vals = frame_vals (frame);

    while (!r_null_p (vars)) {
        if (r_eq_p (r, r_car (vars), var))
            return vals;

        vars = r_cdr (vars);
        vals = r_cdr (vals);
    }

    return r_env_lookup (r, enclosing_env (env), var);
}

rsexp r_empty_env (RState* r)
{
    rsexp env;

    env = R_NULL;
    ensure (env = env_extend (r, env, R_NULL, R_NULL));

    return env;
}

rsexp r_env_bind_x (RState* r, rsexp env, rsexp var, rsexp val)
{
    rsexp vars;
    rsexp vals;
    rsexp frame;

    frame = first_frame (env);
    vars = frame_vars (frame);
    vals = frame_vals (frame);

    while (!r_null_p (vars)) {
        if (r_eq_p (r, var, r_car (vars))) {
            r_set_car_x (vals, val);
            return env;
        }

        vars = r_cdr (vars);
        vals = r_cdr (vals);
    }

    ensure (frame_bind_x (r, frame, var, val));

    return env;
}

rsexp r_env_assign_x (RState* r, rsexp env, rsexp var, rsexp val)
{
    rsexp vals;

    vals = r_env_lookup (r, env, var);

    if (r_undefined_p (vals)) {
        r_error_code (r, R_ERR_UNBOUND_VAR, var);
        return R_FAILURE;
    }

    r_set_car_x (vals, val);

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
    rsexp env;

    ensure (env = r_empty_env (r));

    ensure (env = init_primitives (r, env, bytevector_primitives));
    ensure (env = init_primitives (r, env, gc_primitives));
    ensure (env = init_primitives (r, env, io_primitives));
    ensure (env = init_primitives (r, env, math_primitives));
    ensure (env = init_primitives (r, env, number_primitives));
    ensure (env = init_primitives (r, env, pair_primitives));
    ensure (env = init_primitives (r, env, read_primitives));
    ensure (env = init_primitives (r, env, string_primitives));

    return env;
}
