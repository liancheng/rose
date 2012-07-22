#include "detail/state.h"
#include "detail/port.h"
#include "rose/error.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/writer.h"

#include <gc/gc.h>
#include <stdarg.h>
#include <string.h>

static RType* r_port_type_info ();

static rsexp r_file_port_new (RState*     state,
                              FILE*       file,
                              char const* name,
                              rint        mode)
{
    RPort* port = GC_NEW (RPort);

    port->type   = r_port_type_info ();
    port->state  = state;
    port->stream = file;
    port->name   = r_string_new (name);
    port->mode   = mode;

    return PORT_TO_SEXP (port);
}

static rsexp r_open_file (RState* state, char const* filename, rint mode)
{
    FILE* file = fopen (filename, (INPUT_PORT == mode) ? "r" : "w");

    if (!file)
        return r_error_new (r_string_new ("cannot open file"),
                            r_string_new (filename));

    return r_file_port_new (state, file, filename, mode);
}

static void r_port_write (rsexp port, rsexp obj)
{
    r_port_printf (port, "#<port %s>", PORT_FROM_SEXP (obj)->name);
}

static RType* r_port_type_info ()
{
    static RType type = {
        .size    = sizeof (RPort),
        .name    = "port",
        .write   = r_port_write,
        .display = r_port_write
    };

    return &type;
}

RState* r_port_get_state (rsexp port)
{
    return PORT_FROM_SEXP (port)->state;
}

rsexp r_open_input_file (RState* state, char const* filename)
{
    return r_open_file (state, filename, INPUT_PORT);
}

rsexp r_open_output_file (RState* state, char const* filename)
{
    return r_open_file (state, filename, OUTPUT_PORT);
}

rsexp r_open_input_string (RState* state, char const* string)
{
    FILE* file = fmemopen ((void*) string, strlen (string), "r");
    return r_file_port_new (state, file, "(string-input)", INPUT_PORT);
}

rsexp r_stdin_port (RState* state)
{
    return r_file_port_new (state, stdin, "(standard-input)", INPUT_PORT);
}

rsexp r_stdout_port (RState* state)
{
    return r_file_port_new (state, stdout, "(standard-output)", OUTPUT_PORT);
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
    return r_boxed_p (obj) &&
           R_SEXP_TYPE (obj) == r_port_type_info ();
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
