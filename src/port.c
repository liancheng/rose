#include "cell.h"

#include "rose/port.h"
#include "rose/string.h"

#include <stdarg.h>
#include <stdio.h>

#define SEXP_TO_PORT(obj)   R_CELL_VALUE (obj).port
#define PORT_TO_FILE(port)  ((FILE*) (SEXP_TO_PORT (port).stream))

static void finalize_port (void* obj, void* client_data)
{
    r_close_port ((rsexp) obj);
}

static rsexp file_port_new (FILE*       file,
                            char const* name,
                            rint        mode,
                            rboolean    close_on_destroy)
{
    R_SEXP_NEW (port, SEXP_PORT);

    SEXP_TO_PORT (port).stream  = file;
    SEXP_TO_PORT (port).name    = r_string_new (name);
    SEXP_TO_PORT (port).mode    = mode;

    if (close_on_destroy)
        GC_REGISTER_FINALIZER ((void*) port, finalize_port, NULL, NULL, NULL);

    return port;
}

rboolean r_port_p (rsexp obj)
{
    return r_cell_p (obj) &&
           r_cell_get_type (obj) == SEXP_PORT;
}

rboolean r_input_port_p (rsexp obj)
{
    return r_port_p (obj) &&
           SEXP_TO_PORT (obj).mode == INPUT_PORT;
}

rboolean r_output_port_p (rsexp obj)
{
    return r_port_p (obj) &&
           SEXP_TO_PORT (obj).mode == OUTPUT_PORT;
}

static rsexp open_file (char const* filename, rint mode)
{
    FILE* file;
    char* mode_str;

    mode_str = (INPUT_PORT == mode) ? "r" : "w";
    file = fopen (filename, mode_str);

    if (!file)
        return r_error (r_string_new ("cannot open file"),
                        r_string_new (filename));

    return file_port_new (file, filename, mode, TRUE);
}

rsexp r_open_input_file (char const* filename)
{
    return open_file (filename, INPUT_PORT);
}

rsexp r_open_output_file (char const* filename)
{
    return open_file (filename, OUTPUT_PORT);
}

rsexp r_stdin_port ()
{
    return file_port_new (stdin, "(standard-input)", INPUT_PORT, FALSE);
}

rsexp r_stdout_port ()
{
    return file_port_new (stdout, "(standard-output)", OUTPUT_PORT, FALSE);
}

void r_close_port (rsexp port)
{
    fclose (PORT_TO_FILE (port));
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

void r_write_port (rsexp port, rsexp obj, rsexp context)
{
    r_port_printf (port, "#<port %s>", SEXP_TO_PORT (obj).name);
}

void r_display_port (rsexp port, rsexp obj, rsexp context)
{
    r_write_port (port, obj, context);
}

void r_newline (rsexp port)
{
    r_port_puts (port, "\n");
}

void r_write_char (rsexp port, char ch)
{
    fputc (ch, PORT_TO_FILE (port));
}

rboolean r_eof_p (rsexp port)
{
    return 0 != feof (PORT_TO_FILE (port));
}
