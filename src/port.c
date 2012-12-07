#include "detail/state.h"
#include "detail/port.h"
#include "rose/error.h"
#include "rose/memory.h"
#include "rose/string.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define mode_set_p(port, m)     ((port)->mode & (m))
#define set_mode_x(port, m)     ((port)->mode |= (m))
#define clear_mode_x(port, m)   ((port)->mode &= ~(m))

static rsexp make_port (RState*        state,
                        FILE*          stream,
                        rconstcstring  name,
                        RPortMode      mode,
                        rpointer       cookie,
                        RPortClearFunc clear,
                        RPortMarkFunc  mark)
{
    RPort* port = r_object_new (state, RPort, R_PORT_TAG);

    if (!port)
        return R_FALSE;

    port->name = r_string_new (state, name);

    if (r_false_p (port->name))
        return R_FALSE;

    port->state  = state;
    port->stream = stream;
    port->mode   = mode;
    port->cookie = cookie;
    port->clear  = clear;
    port->mark   = mark;

    return port_to_sexp (port);
}

static void write_port (RState* state, rsexp port, rsexp obj)
{
    r_port_format (state, port, "#<port ~a>", port_from_sexp (obj)->name);
}

static void destruct_port (RState* state, RObject* obj)
{
    r_close_port (port_to_sexp (r_cast (RPort*, obj)));
}

static void input_string_port_mark (RState* state, rpointer cookie)
{
    // r_gc_mark (r_cast (rsexp, cookie));
}

typedef struct {
    rcstring buffer;
    rsize    size;
}
ROutStringCookie;

static void output_string_port_clear (RState* state, rpointer cookie)
{
    ROutStringCookie* c = r_cast (ROutStringCookie*, cookie);

    free (c->buffer);
    r_free (state, c);
}

static rbool output_string_port_p (rsexp port)
{
    return mode_set_p (port_from_sexp (port), MODE_STRING_IO | MODE_OUTPUT);
}

void init_port_type_info (RState* state)
{
    RTypeInfo* type = r_new0 (state, RTypeInfo);

    type->size         = sizeof (RPort);
    type->name         = "port";
    type->ops.write    = write_port;
    type->ops.display  = write_port;
    type->ops.eqv_p    = NULL;
    type->ops.equal_p  = NULL;
    type->ops.mark     = NULL;
    type->ops.destruct = destruct_port;

    state->builtin_types [R_PORT_TAG] = type;
}

RState* r_port_get_state (rsexp port)
{
    return port_from_sexp (port)->state;
}

rsexp r_open_input_file (RState* state, rconstcstring filename)
{
    FILE* stream = fopen (filename, "r");

    if (!stream)
        return R_FALSE;

    return make_port (state, stream, filename, MODE_INPUT, NULL, NULL, NULL);
}

rsexp r_open_output_file (RState* state, rconstcstring filename)
{
    FILE* stream = fopen (filename, "w");

    if (!stream)
        return R_FALSE;

    return make_port (state, stream, filename, MODE_OUTPUT, NULL, NULL, NULL);
}

rsexp r_open_input_string (RState* state, rsexp string)
{
    rpointer input  = r_cast (rpointer, r_string_to_cstr (string));
    rsize    size   = r_string_byte_count (string);
    FILE*    stream = fmemopen (input, size, "r");

    return make_port (state, stream, "(input-string-port)",
                      MODE_INPUT | MODE_STRING_IO, r_cast (rpointer, string),
                      NULL, input_string_port_mark);
}

rsexp r_open_output_string (RState* state)
{
    ROutStringCookie* cookie;
    FILE* stream;

    cookie = r_new0 (state, ROutStringCookie);

    if (!cookie)
        return R_FALSE;

    stream = open_memstream (&cookie->buffer, &cookie->size);

    if (!stream) {
        r_inherit_errno_x (state, errno);
        r_free (state, cookie);
        return R_FALSE;
    }

    return make_port (state, stream, "(output-string-port)",
                      MODE_OUTPUT | MODE_STRING_IO | MODE_FLUSH,
                      r_cast (rpointer, cookie),
                      output_string_port_clear, NULL);
}

rsexp r_get_output_string (RState* state, rsexp port)
{
    RPort*            port_ptr;
    ROutStringCookie* cookie;

    assert (output_string_port_p (port));

    port_ptr = port_from_sexp (port);
    cookie   = r_cast (ROutStringCookie*, port_ptr->cookie);

    return cookie->buffer
           ? r_string_new (state, cookie->buffer)
           : r_string_new (state, "");
}

rsexp r_stdin_port (RState* state)
{
    return make_port (state, stdin, "(standard-input)",
                      MODE_INPUT | MODE_DONT_CLOSE, NULL, NULL, NULL);
}

rsexp r_stdout_port (RState* state)
{
    return make_port (state, stdout, "(standard-output)",
                      MODE_OUTPUT | MODE_DONT_CLOSE, NULL, NULL, NULL);
}

rsexp r_stderr_port (RState* state)
{
    return make_port (state, stderr, "(standard-error)",
                      MODE_OUTPUT | MODE_DONT_CLOSE, NULL, NULL, NULL);
}

rsexp r_port_get_name (rsexp port)
{
    return port_from_sexp (port)->name;
}

void r_close_port (rsexp port)
{
    RPort* p = port_from_sexp (port);

    if (mode_set_p (p, MODE_DONT_CLOSE))
        return;

    fclose (p->stream);
    set_mode_x (port_from_sexp (port), MODE_CLOSED);
}

rbool r_eof_p (rsexp port)
{
    return 0 != feof (port_to_file (port));
}

rbool r_port_p (rsexp obj)
{
    return r_type_tag (obj) == R_PORT_TAG;
}

rbool r_input_port_p (rsexp obj)
{
    return r_port_p (obj)
        && mode_set_p (port_from_sexp (obj), MODE_INPUT);
}

rbool r_output_port_p (rsexp obj)
{
    return r_port_p (obj)
        && mode_set_p (port_from_sexp (obj), MODE_OUTPUT);
}

rint r_port_vprintf (rsexp         port,
                     rconstcstring format,
                     va_list       args)
{
    rint ret = vfprintf (port_to_file (port), format, args);

    if (mode_set_p (port_from_sexp (port), MODE_FLUSH))
        fflush (port_to_file (port));

    return ret;
}

rint r_port_printf (rsexp port, rconstcstring format, ...)
{
    va_list args;
    rint    ret;

    va_start (args, format);
    ret = r_port_vprintf (port, format, args);
    va_end (args);

    return ret;
}

rcstring r_port_gets (rsexp port, rcstring dest, rint size)
{
    return fgets (dest, size, port_to_file (port));
}

rint r_port_puts (rsexp port, rconstcstring str)
{
    rint ret = fputs (str, port_to_file (port));

    if (mode_set_p (port_from_sexp (port), MODE_FLUSH))
        fflush (port_to_file (port));

    return ret;
}

rchar r_read_char (rsexp port)
{
    return (rchar) fgetc (port_to_file (port_from_sexp (port)));
}

void r_write_char (rsexp port, rchar ch)
{
    fputc (ch, port_to_file (port));

    if (mode_set_p (port_from_sexp (port), MODE_FLUSH))
        fflush (port_to_file (port));
}

void r_port_vformat (RState*       state,
                     rsexp         port,
                     rconstcstring format,
                     va_list       args)
{
    rconstcstring pos;

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
                r_port_display (state, port, va_arg (args, rsexp));
                break;

            case 's':
                r_port_write (state, port, va_arg (args, rsexp));
                break;
        }
    }
}

void r_port_format (RState* state, rsexp port, rconstcstring format, ...)
{
    va_list args;

    va_start (args, format);
    r_port_vformat (state, port, format, args);
    va_end (args);
}

void r_format (RState* state, rconstcstring format, ...)
{
    va_list args;

    va_start (args, format);
    r_port_vformat (state, r_current_output_port (state), format, args);
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

rsexp r_current_error_port (RState* state)
{
    return state->current_error_port;
}

void r_set_current_input_port_x (RState* state, rsexp port)
{
    state->current_input_port = port;
}

void r_set_current_output_port_x (RState* state, rsexp port)
{
    state->current_output_port = port;
}

void r_set_current_error_port_x (RState* state, rsexp port)
{
    state->current_error_port = port;
}

void r_port_write (RState* state, rsexp port, rsexp obj)
{
    RTypeInfo* type_info = r_type_info (state, obj);

    if (type_info)
        type_info->ops.write (state, port, obj);
}

void r_port_display (RState* state, rsexp port, rsexp obj)
{
    RTypeInfo* type_info = r_type_info (state, obj);

    if (type_info)
        type_info->ops.display (state, port, obj);
}

void r_write (RState* state, rsexp obj)
{
    r_port_write (state, r_current_output_port (state), obj);
}

void r_display (RState* state, rsexp obj)
{
    r_port_display (state, r_current_output_port (state), obj);
}
