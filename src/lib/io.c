#include "detail/primitive.h"
#include "rose/error.h"
#include "rose/io.h"
#include "rose/pair.h"

static rsexp np_write (RState* r, rsexp args)
{
    rsexp datum;
    rsexp port;

    r_match_args (r, args, 1, 1, FALSE, &datum, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (r);

    return r_port_write (r, port, datum);
}

static rsexp np_display (RState* r, rsexp args)
{
    rsexp datum;
    rsexp port;

    r_match_args (r, args, 1, 1, FALSE, &datum, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (r);

    if (!r_output_port_p (port)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    return r_port_display (r, port, datum);
}

static rsexp np_write_char (RState* r, rsexp args)
{
    rsexp ch;
    rsexp port;

    r_match_args (r, args, 1, 1, FALSE, &ch, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (r);

    if (!r_output_port_p (port)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    return r_port_write_char (r, port, r_char_from_sexp (ch));
}

static rsexp np_newline (RState* r, rsexp args)
{
    rsexp port;

    r_match_args (r, args, 0, 1, FALSE, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (r);

    if (!r_output_port_p (port)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    return r_port_write_char (r, port, '\n');
}

static rsexp np_read_char (RState* r, rsexp args)
{
    rsexp port;

    r_match_args (r, args, 0, 1, FALSE, &port);

    if (r_undefined_p (port))
        port = r_current_input_port (r);

    if (!r_input_port_p (port)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    return r_port_read_char (r, port);
}

static rsexp np_current_input_port (RState* r, rsexp args)
{
    return r_current_input_port (r);
}

static rsexp np_current_output_port (RState* r, rsexp args)
{
    return r_current_output_port (r);
}

static rsexp np_current_error_port (RState* r, rsexp args)
{
    return r_current_error_port (r);
}

static rsexp np_set_current_input_port_x (RState* r, rsexp args)
{
    r_set_current_input_port_x (r, r_car (args));
    return R_UNSPECIFIED;
}

static rsexp np_set_current_output_port_x (RState* r, rsexp args)
{
    r_set_current_output_port_x (r, r_car (args));
    return R_UNSPECIFIED;
}

static rsexp np_set_current_error_port_x (RState* r, rsexp args)
{
    r_set_current_error_port_x (r, r_car (args));
    return R_UNSPECIFIED;
}

static rsexp np_open_input_string (RState* r, rsexp args)
{
    return r_open_input_string (r, r_car (args));
}

static rsexp np_open_output_string (RState* r, rsexp args)
{
    return r_open_output_string (r);
}

static rsexp np_get_output_string (RState* r, rsexp args)
{
    return r_get_output_string (r, r_car (args));
}

static rsexp np_port_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_port_p (r_car (args)));
}

static rsexp np_input_port_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_input_port_p (r_car (args)));
}

static rsexp np_output_port_p (RState* r, rsexp args)
{
    return r_bool_to_sexp (r_output_port_p (r_car (args)));
}

static rsexp np_close_port (RState* r, rsexp args)
{
    rsexp port = r_car (args);

    if (!r_port_p (port)) {
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, port);
        return R_FAILURE;
    }

    r_close_port (port);

    return R_UNSPECIFIED;
}

RPrimitiveDesc io_primitives [] = {
    { "current-input-port",       np_current_input_port,        0, 0, FALSE },
    { "current-output-port",      np_current_output_port,       0, 0, FALSE },
    { "current-error-port",       np_current_error_port,        0, 0, FALSE },
    { "set-current-input-port!",  np_set_current_input_port_x,  1, 0, FALSE },
    { "set-current-output-port!", np_set_current_output_port_x, 1, 0, FALSE },
    { "set-current-error-port!",  np_set_current_error_port_x,  1, 0, FALSE },
    { "open-input-string",        np_open_input_string,         1, 0, FALSE },
    { "open-output-string",       np_open_output_string,        0, 0, FALSE },
    { "get-output-string",        np_get_output_string,         1, 0, FALSE },
    { "input-port?",              np_input_port_p,              1, 0, FALSE },
    { "output-port?",             np_output_port_p,             1, 0, FALSE },
    { "port?",                    np_port_p,                    1, 0, FALSE },
    { "display",                  np_display,                   1, 1, FALSE },
    { "write",                    np_write,                     1, 1, FALSE },
    { "newline",                  np_newline,                   0, 1, FALSE },
    { "read-char",                np_read_char,                 0, 1, FALSE },
    { "write-char",               np_write_char,                1, 1, FALSE },
    { "close-port",               np_close_port,                1, 1, FALSE },
    { NULL },
};
