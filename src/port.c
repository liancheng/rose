#include "boxed.h"

#include "rose/port.h"

#include <stdarg.h>
#include <stdio.h>

static void file_port_finalize(void* obj, void* client_data)
{
    r_file_port_close((rsexp)obj);
}

static rsexp file_port_new(FILE*       file,
                           char const* name,
                           rboolean    close_on_destroy)
{
    R_SEXP_NEW(port, SEXP_PORT);

    R_BOXED_VALUE(port).port.native_stream = file;
    R_BOXED_VALUE(port).port.name = name;

    if (close_on_destroy)
        GC_REGISTER_FINALIZER(
                (void*)port, file_port_finalize, NULL, NULL, NULL);

    return port;
}

rboolean r_port_p(rsexp sexp)
{
    return R_BOXED_P(sexp) &&
           r_boxed_get_type(sexp) == SEXP_PORT;
}

rsexp r_file_input_port_new(FILE*       file,
                            char const* name,
                            rboolean    close_on_destroy)
{
    return file_port_new(file, name, close_on_destroy);
}

rsexp r_file_output_port_new(FILE*       file,
                             char const* name,
                             rboolean    close_on_destroy)
{
    return file_port_new(file, name, close_on_destroy);
}

void r_file_port_close(rsexp port)
{
    fclose(R_BOXED_VALUE(port).port.native_stream);
}

void r_set_current_input_port(rsexp port, rsexp context)
{
    r_context_set(context, CTX_CURRENT_INPUT_PORT, port);
}

rsexp r_get_current_input_port(rsexp context)
{
    return r_context_get(context, CTX_CURRENT_INPUT_PORT);
}

void r_set_current_output_port(rsexp port, rsexp context)
{
    r_context_set(context, CTX_CURRENT_OUTPUT_PORT, port);
}

rsexp r_get_current_output_port(rsexp context)
{
    return r_context_get(context, CTX_CURRENT_OUTPUT_PORT);
}

rint r_pprintf(rsexp port, char const* format, ...)
{
    va_list args;
    rint    ret;
    FILE*   out;

    va_start(args, format);
    out = R_BOXED_VALUE(port).port.native_stream;
    ret = vfprintf(out, format, args);
    va_end(args);

    return ret;
}

char* r_pgets(char* dest, rint size, rsexp port)
{
    FILE* out = R_BOXED_VALUE(port).port.native_stream;
    return fgets(dest, size, out);
}
