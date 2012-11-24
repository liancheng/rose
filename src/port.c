#include "detail/state.h"
#include "detail/port.h"
#include "rose/error.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/writer.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static RTypeDescriptor* port_type_info ();

static rsexp make_file_port (RState*     state,
                             FILE*       file,
                             char const* name,
                             rint        mode)
{
    RPort* port = r_cast (RPort*,
                          r_object_new (state,
                                        R_TYPE_PORT,
                                        port_type_info ()));

    port->state  = state;
    port->stream = file;
    port->name   = r_string_new (state, name);
    port->mode   = mode;

    return PORT_TO_SEXP (port);
}

static rsexp open_file (RState* state, char const* filename, rint mode)
{
    FILE* file = fopen (filename, (INPUT_PORT == mode) ? "r" : "w");

    if (!file)
        return r_error_new (state,
                            r_string_new (state, "cannot open file"),
                            r_string_new (state, filename));

    return make_file_port (state, file, filename, mode);
}

static void write_port (rsexp port, rsexp obj)
{
    r_port_printf (port, "#<port %s>", PORT_FROM_SEXP (obj)->name);
}

static void destruct_port (RState* state, RObject* obj)
{
}

static RTypeDescriptor* port_type_info ()
{
    static RTypeDescriptor type = {
        .size = sizeof (RPort),
        .name = "port",
        .ops = {
            .write    = write_port,
            .display  = write_port,
            .eqv_p    = NULL,
            .equal_p  = NULL,
            .mark     = NULL,
            .destruct = destruct_port
        }
    };

    return &type;
}

RState* r_port_get_state (rsexp port)
{
    return PORT_FROM_SEXP (port)->state;
}

rsexp r_open_input_file (RState* state, char const* filename)
{
    return open_file (state, filename, INPUT_PORT);
}

rsexp r_open_output_file (RState* state, char const* filename)
{
    return open_file (state, filename, OUTPUT_PORT);
}

rsexp r_open_input_string (RState* state, char const* string)
{
    FILE* file = fmemopen (r_cast (void*, string), strlen (string), "r");
    return make_file_port (state, file, "(string-input)", INPUT_PORT);
}

rsexp r_stdin_port (RState* state)
{
    return make_file_port (state, stdin, "(standard-input)", INPUT_PORT);
}

rsexp r_stdout_port (RState* state)
{
    return make_file_port (state, stdout, "(standard-output)", OUTPUT_PORT);
}

rsexp r_port_get_name (rsexp port)
{
    return PORT_FROM_SEXP (port)->name;
}

void r_close_input_port (rsexp port)
{
    r_close_port (port);
}

void r_close_output_port (rsexp port)
{
    r_close_port (port);
}

void r_close_port (rsexp port)
{
    fclose (PORT_TO_FILE (port));
}

rbool r_eof_p (rsexp port)
{
    return 0 != feof (PORT_TO_FILE (port));
}

rbool r_port_p (rsexp obj)
{
    return r_type_tag (obj) == R_TYPE_PORT;
}

rbool r_input_port_p (rsexp obj)
{
    return r_port_p (obj) &&
           PORT_FROM_SEXP (obj)->mode == INPUT_PORT;
}

rbool r_output_port_p (rsexp obj)
{
    return r_port_p (obj) &&
           PORT_FROM_SEXP (obj)->mode == OUTPUT_PORT;
}

rint r_port_printf (rsexp port, char const* format, ...)
{
    va_list args;
    rint ret;

    va_start (args, format);
    ret = vfprintf (PORT_TO_FILE (port), format, args);
    va_end (args);

    return ret;
}

char* r_port_gets (rsexp port, char* dest, rint size)
{
    return fgets (dest, size, PORT_TO_FILE (port));
}

rint r_port_puts (rsexp port, char const* str)
{
    return fputs (str, PORT_TO_FILE (port));
}

char r_read_char (rsexp port)
{
    return (char) fgetc (PORT_TO_FILE (PORT_FROM_SEXP (port)));
}

void r_write_char (rsexp port, char ch)
{
    fputc (ch, PORT_TO_FILE (port));
}

void r_format (rsexp port, char const* format, ...)
{
    va_list args;
    char const* pos;

    va_start (args, format);

    for (pos = format; *pos; ++pos) {
        if ('~' != *pos) {
            r_write_char (port, *pos);
            continue;
        }

        switch (*++pos) {
            case '~':
                r_write_char (port, '~');
                break;

            case '%':
                r_write_char (port, '\n');
                break;

            case 'a':
                r_display (port, va_arg (args, rsexp));
                break;

            case 's':
                r_write (port, va_arg (args, rsexp));
                break;
        }
    }

    va_end (args);
}

rsexp r_current_input_port (RState* state)
{
    return state->current_input_port;
}

rsexp r_current_output_port (RState* state)
{
    return state->current_output_port;
}

void r_set_current_input_port_x (RState* state, rsexp port)
{
    state->current_input_port = port;
}

void r_set_current_output_port_x (RState* state, rsexp port)
{
    state->current_output_port = port;
}
