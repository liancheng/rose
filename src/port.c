#include "detail/context.h"
#include "detail/sexp.h"
#include "rose/error.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/writer.h"

#include <gc/gc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

enum {
    INPUT_PORT,
    OUTPUT_PORT,
};

struct RPort {
    RType*    type;
    RContext* context;
    FILE*     stream;
    rint      mode;
    rsexp     name;
};

#define SEXP_TO_PORT(obj)    (*((RPort*) (obj)))
#define SEXP_FROM_PORT(port) ((rsexp) (port))
#define PORT_TO_FILE(obj)    ((FILE*) (SEXP_TO_PORT (obj).stream))

static RType* r_port_type_info ();

static void r_port_finalize (void* obj, void* client_data)
{
    r_close_port ((rsexp) obj);
}

static rsexp r_file_port_new (FILE*       file,
                              char const* name,
                              rint        mode,
                              rboolean    close_on_destroy,
                              RContext*   context)
{
    RPort* port = GC_NEW (RPort);

    port->type    = r_port_type_info ();
    port->context = context;
    port->stream  = file;
    port->name    = r_string_new (name);
    port->mode    = mode;

    if (close_on_destroy)
        GC_REGISTER_FINALIZER ((void*) port, r_port_finalize, NULL, NULL, NULL);

    return SEXP_FROM_PORT (port);
}

static rsexp r_open_file (char const* filename, rint mode, RContext* context)
{
    FILE* file;
    char* mode_str;

    mode_str = (INPUT_PORT == mode) ? "r" : "w";
    file = fopen (filename, mode_str);

    if (!file)
        return r_error_new (r_string_new ("cannot open file"),
                            r_string_new (filename));

    return r_file_port_new (file, filename, mode, TRUE, context);
}

static void r_port_write (rsexp port, rsexp obj, RContext* context)
{
    r_port_printf (port, "#<port %s>", SEXP_TO_PORT (obj).name);
}

static RType* r_port_type_info ()
{
    static RType* type = NULL;

    if (!type) {
        type = GC_NEW (RType);

        type->cell_size  = sizeof (RPort);
        type->name       = "port";
        type->write_fn   = r_port_write;
        type->display_fn = r_port_write;
    }

    return type;
}

RContext* r_port_get_context (rsexp port)
{
    return SEXP_TO_PORT (port).context;
}

rsexp r_open_input_file (char const* filename, RContext* context)
{
    return r_open_file (filename, INPUT_PORT, context);
}

rsexp r_open_output_file (char const* filename, RContext* context)
{
    return r_open_file (filename, OUTPUT_PORT, context);
}

rsexp r_open_input_string (char const* string, RContext* context)
{
    FILE* file = fmemopen ((void*) string, strlen (string), "r");
    return r_file_port_new (file,
                            "(string-input)",
                            INPUT_PORT,
                            TRUE,
                            context);
}

rsexp r_stdin_port (RContext* context)
{
    return r_file_port_new (stdin,
                            "(standard-input)",
                            INPUT_PORT,
                            FALSE,
                            context);
}

rsexp r_stdout_port (RContext* context)
{
    return r_file_port_new (stdout,
                            "(standard-output)",
                            OUTPUT_PORT,
                            FALSE,
                            context);
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

rboolean r_eof_p (rsexp port)
{
    return 0 != feof (PORT_TO_FILE (port));
}

rboolean r_port_p (rsexp obj)
{
    return r_cell_p (obj) &&
           R_CELL_TYPE (obj) == r_port_type_info ();
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
    rsexp arg;

    va_start (args, format);

    for (pos = format; *pos; ++pos) {
        if ('~' != *pos)
            r_write_char (port, *pos);

        switch (*++pos) {
            case '~':
                r_write_char (port, '~');
                break;

            case '%':
                r_write_char (port, '\n');
                break;

            case 'a':
                arg = va_arg (args, rsexp);
                r_display (port, arg);
                break;

            case 's':
                arg = va_arg (args, rsexp);
                r_write (port, arg);
                break;
        }
    }

    va_end (args);
}

rsexp r_current_input_port (RContext* context)
{
    return context->current_input_port;
}

rsexp r_current_output_port (RContext* context)
{
    return context->current_output_port;
}

void r_set_current_input_port_x (rsexp port, RContext* context)
{
    context->current_input_port = port;
}

void r_set_current_output_port_x (rsexp port, RContext* context)
{
    context->current_output_port = port;
}
