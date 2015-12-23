#include "rose/error.h"
#include "rose/io.h"
#include "rose/pair.h"
#include "rose/primitive.h"
#include "rose/string.h"

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

    r_check_arg (r, port, r_output_port_p, R_ERR_WRONG_TYPE_ARG);

    return r_port_display (r, port, datum);
}

static rsexp np_write_char (RState* r, rsexp args)
{
    rsexp ch;
    rsexp port;

    r_match_args (r, args, 1, 1, FALSE, &ch, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (r);

    r_check_arg (r, port, r_output_port_p, R_ERR_WRONG_TYPE_ARG);

    return r_port_write_char (r, port, r_char_from_sexp (ch));
}

static rsexp np_newline (RState* r, rsexp args)
{
    rsexp port;

    r_match_args (r, args, 0, 1, FALSE, &port);

    if (r_undefined_p (port))
        port = r_current_output_port (r);

    r_check_arg (r, port, r_output_port_p, R_ERR_WRONG_TYPE_ARG);

    return r_port_write_char (r, port, '\n');
}

static rsexp np_read_char (RState* r, rsexp args)
{
    rsexp port;

    r_match_args (r, args, 0, 1, FALSE, &port);

    if (r_undefined_p (port))
        port = r_current_input_port (r);

    r_check_arg (r, port, r_output_port_p, R_ERR_WRONG_TYPE_ARG);

    return r_port_read_char (r, port);
}

static rsexp np_current_input_port (RState* r, rsexp args)
{
    r_match_args (r, args, 0, 0, FALSE);
    return r_current_input_port (r);
}

static rsexp np_current_output_port (RState* r, rsexp args)
{
    r_match_args (r, args, 0, 0, FALSE);
    return r_current_output_port (r);
}

static rsexp np_current_error_port (RState* r, rsexp args)
{
    r_match_args (r, args, 0, 0, FALSE);
    return r_current_error_port (r);
}

static rsexp np_set_current_input_port_x (RState* r, rsexp args)
{
    rsexp port;
    r_match_args (r, args, 1, 0, FALSE, &port);
    r_check_arg (r, port, r_input_port_p, R_ERR_WRONG_TYPE_ARG);
    r_set_current_input_port_x (r, port);
    return R_UNSPECIFIED;
}

static rsexp np_set_current_output_port_x (RState* r, rsexp args)
{
    rsexp port;
    r_match_args (r, args, 1, 0, FALSE, &port);
    r_check_arg (r, port, r_output_port_p, R_ERR_WRONG_TYPE_ARG);
    r_set_current_output_port_x (r, port);
    return R_UNSPECIFIED;
}

static rsexp np_set_current_error_port_x (RState* r, rsexp args)
{
    rsexp port;
    r_match_args (r, args, 1, 0, FALSE, &port);
    r_check_arg (r, port, r_output_port_p, R_ERR_WRONG_TYPE_ARG);
    r_set_current_error_port_x (r, port);
    return R_UNSPECIFIED;
}

static rsexp np_open_input_string (RState* r, rsexp args)
{
    rsexp string;
    r_match_args (r, args, 1, 0, FALSE, &string);
    r_check_arg (r, string, r_string_p, R_ERR_WRONG_TYPE_ARG);
    return r_open_input_string (r, string);
}

static rsexp np_open_output_string (RState* r, rsexp args)
{
    r_match_args (r, args, 0, 0, FALSE);
    return r_open_output_string (r);
}

static rsexp np_get_output_string (RState* r, rsexp args)
{
    rsexp obj;
    r_match_args (r, args, 1, 0, FALSE, &obj);
    return r_get_output_string (r, obj);
}

static rsexp np_port_p (RState* r, rsexp args)
{
    rsexp obj;
    r_match_args (r, args, 1, 0, FALSE, &obj);
    return r_bool_to_sexp (r_port_p (obj));
}

static rsexp np_input_port_p (RState* r, rsexp args)
{
    rsexp obj;
    r_match_args (r, args, 1, 0, FALSE, &obj);
    return r_bool_to_sexp (r_input_port_p (obj));
}

static rsexp np_output_port_p (RState* r, rsexp args)
{
    rsexp obj;
    r_match_args (r, args, 1, 0, FALSE, &obj);
    return r_bool_to_sexp (r_output_port_p (obj));
}

static rsexp np_close_port (RState* r, rsexp args)
{
    rsexp port;
    r_match_args (r, args, 1, 0, FALSE, &port);

    r_check_arg (r, port, r_port_p, R_ERR_WRONG_TYPE_ARG);
    r_close_port (port);

    return R_UNSPECIFIED;
}

const RPrimitiveDesc io_primitives [] = {
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
