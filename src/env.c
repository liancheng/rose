#include "detail/env.h"
#include "rose/eq.h"
#include "rose/error.h"
#include "rose/native.h"
#include "rose/pair.h"
#include "rose/port.h"
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

rsexp np_cons (RState* state, rsexp args)
{
    rsexp car, cdr;
    r_match_args (state, args, 2, 0, FALSE, &car, &cdr);
    return r_cons (state, car, cdr);
}

rsexp np_pair_p (RState* state, rsexp args)
{
    return r_bool_to_sexp (r_pair_p (r_car (args)));
}

rsexp np_null_p (RState* state, rsexp args)
{
    return r_bool_to_sexp (r_null_p (r_car (args)));
}

rsexp np_car (RState* state, rsexp args)
{
    rsexp pair = r_car (args);

    if (!r_pair_p (pair)) {
        r_error_code (state, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    return r_car (pair);
}

rsexp np_cdr (RState* state, rsexp args)
{
    rsexp pair = r_car (args);

    if (!r_pair_p (pair)) {
        r_error_code (state, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    return r_cdr (pair);
}

rsexp np_set_car_x (RState* state, rsexp args)
{
    rsexp pair = r_car (args);
    rsexp car = r_cadr (args);

    if (!r_pair_p (pair)) {
        r_error_code (state, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    r_set_car_x (pair, car);

    return R_UNSPECIFIED;
}

rsexp np_set_cdr_x (RState* state, rsexp args)
{
    rsexp pair = r_car (args);
    rsexp cdr = r_cadr (args);

    if (!r_pair_p (pair)) {
        r_error_code (state, R_ERR_WRONG_TYPE_ARG, pair);
        return R_FAILURE;
    }

    r_set_cdr_x (pair, cdr);

    return R_UNDEFINED;
}

rsexp np_list (RState* state, rsexp args)
{
    return args;
}

rsexp np_reverse (RState* state, rsexp args)
{
    return r_reverse (state, r_car (args));
}

rsexp np_reverse_x (RState* state, rsexp args)
{
    return r_reverse_x (state, r_car (args));
}

rsexp np_append_x (RState* state, rsexp args)
{
    return r_append_x (state, r_car (args), r_cadr (args));
}

rsexp np_write (RState* state, rsexp args)
{
    rsexp datum;
    rsexp port;

    r_match_args (state, args, 1, 1, FALSE, &datum, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (state);

    return r_port_write (state, port, datum);
}

rsexp np_display (RState* state, rsexp args)
{
    rsexp datum;
    rsexp port;

    r_match_args (state, args, 1, 1, FALSE, &datum, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (state);

    if (!r_output_port_p (port)) {
        r_error_code (state, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    return r_port_display (state, port, datum);
}

rsexp np_write_char (RState* state, rsexp args)
{
    rsexp ch;
    rsexp port;

    r_match_args (state, args, 1, 1, FALSE, &ch, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (state);

    if (!r_output_port_p (port)) {
        r_error_code (state, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    return r_port_write_char (state, port, r_char_from_sexp (ch));
}

rsexp np_newline (RState* state, rsexp args)
{
    rsexp port;

    r_match_args (state, args, 0, 1, FALSE, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (state);

    if (!r_output_port_p (port)) {
        r_error_code (state, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    return r_port_write_char (state, port, '\n');
}

rsexp np_read_char (RState* state, rsexp args)
{
    rsexp port;

    r_match_args (state, args, 0, 1, FALSE, &port);

    if (r_undefined_p (port))
        port = r_current_input_port (state);

    if (!r_input_port_p (port)) {
        r_error_code (state, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    return r_port_read_char (state, port);
}

rsexp np_current_input_port (RState* state, rsexp args)
{
    return r_current_input_port (state);
}

rsexp np_current_output_port (RState* state, rsexp args)
{
    return r_current_output_port (state);
}

rsexp np_current_error_port (RState* state, rsexp args)
{
    return r_current_error_port (state);
}

rsexp np_set_current_input_port_x (RState* state, rsexp args)
{
    r_set_current_input_port_x (state, r_car (args));
    return R_UNSPECIFIED;
}

rsexp np_set_current_output_port_x (RState* state, rsexp args)
{
    r_set_current_output_port_x (state, r_car (args));
    return R_UNSPECIFIED;
}

rsexp np_set_current_error_port_x (RState* state, rsexp args)
{
    r_set_current_error_port_x (state, r_car (args));
    return R_UNSPECIFIED;
}

rsexp np_open_input_string (RState* state, rsexp args)
{
    return r_open_input_string (state, r_car (args));
}

rsexp np_open_output_string (RState* state, rsexp args)
{
    return r_open_output_string (state);
}

rsexp np_get_output_string (RState* state, rsexp args)
{
    return r_get_output_string (state, r_car (args));
}

void bind_native (RState* state,
                  rsexp* env,
                  rconstcstring name,
                  RNativeProc proc,
                  rsize required,
                  rsize optional,
                  rbool rest_p)
{
    rsexp name_id = r_symbol_new (state, name);
    *env = r_env_bind_x (state, *env, name_id,
            r_native_new (state, name, proc, required, optional, rest_p));
}

rsexp default_env (RState* state)
{
    rsexp env = r_empty_env (state);

    bind_native (state, &env, "cons",     np_cons,      2, 0, FALSE);
    bind_native (state, &env, "pair?",    np_pair_p,    1, 0, FALSE);
    bind_native (state, &env, "null?",    np_null_p,    1, 0, FALSE);
    bind_native (state, &env, "car",      np_car,       1, 0, FALSE);
    bind_native (state, &env, "cdr",      np_cdr,       1, 0, FALSE);
    bind_native (state, &env, "set-car!", np_set_car_x, 2, 0, FALSE);
    bind_native (state, &env, "set-cdr!", np_set_cdr_x, 2, 0, FALSE);
    bind_native (state, &env, "list",     np_list,      0, 0, TRUE);
    bind_native (state, &env, "reverse",  np_reverse,   1, 0, FALSE);
    bind_native (state, &env, "reverse!", np_reverse_x, 1, 0, FALSE);
    bind_native (state, &env, "append!",  np_append_x,  2, 0, FALSE);

    bind_native (state, &env, "current-input-port",
                 np_current_input_port, 0, 0, FALSE);
    bind_native (state, &env, "current-output-port",
                 np_current_output_port, 0, 0, FALSE);
    bind_native (state, &env, "current-error-port",
                 np_current_error_port, 0, 0, FALSE);

    bind_native (state, &env, "set-current-input-port!",
                 np_set_current_input_port_x, 1, 0, FALSE);
    bind_native (state, &env, "set-current-output-port!",
                 np_set_current_output_port_x, 1, 0, FALSE);
    bind_native (state, &env, "set-current-error-port!",
                 np_set_current_error_port_x, 1, 0, FALSE);

    bind_native (state, &env, "open-input-string",
                 np_open_input_string, 1, 0, FALSE);
    bind_native (state, &env, "open-output-string",
                 np_open_output_string, 0, 0, FALSE);
    bind_native (state, &env, "get-output-string",
                 np_get_output_string, 1, 0, FALSE);

    bind_native (state, &env, "display",    np_display,    1, 1, FALSE);
    bind_native (state, &env, "write",      np_write,      1, 1, FALSE);
    bind_native (state, &env, "newline",    np_newline,    0, 1, FALSE);
    bind_native (state, &env, "read-char",  np_read_char,  0, 1, FALSE);
    bind_native (state, &env, "write-char", np_write_char, 1, 1, FALSE);

    return env;
}
