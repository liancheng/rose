#include "detail/error.h"
#include "detail/port.h"
#include "detail/state.h"
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
    rsexp  res;
    RPort* port;

    port = r_object_new (state, RPort, R_PORT_TAG);

    if (!port) {
        res = R_FAILURE;
        r_last_error (state);
        goto exit;
    }

    port->name = r_string_new (state, name);

    if (r_failure_p (port->name)) {
        res = R_FAILURE;
        goto exit;
    }

    port->state  = state;
    port->stream = stream;
    port->mode   = mode;
    port->cookie = cookie;
    port->clear  = clear;
    port->mark   = mark;

    res = port_to_sexp (port);

exit:
    return res;
}

static rsexp write_port (RState* state, rsexp port, rsexp obj)
{
    return r_port_format (state, port,
            "#<port ~a>", port_from_sexp (obj)->name);
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

static rbool need_flush_p (rsexp port)
{
    return mode_set_p (port_from_sexp (port), MODE_FLUSH);
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

rsexp r_open_input_file (RState* state, rconstcstring filename)
{
    FILE* stream;
    rsexp res;

    stream= fopen (filename, "r");

    if (!stream) {
        res = R_FAILURE;
        r_inherit_errno_x (state, errno);
        goto exit;
    }

    res = make_port (state, stream, filename, MODE_INPUT, NULL, NULL, NULL);

    if (r_failure_p (res))
        fclose (stream);

exit:
    return res;
}

rsexp r_open_output_file (RState* state, rconstcstring filename)
{
    FILE* stream;
    rsexp res;

    stream = fopen (filename, "w");

    if (!stream) {
        res = R_FAILURE;
        r_inherit_errno_x (state, errno);
        goto exit;
    }

    res = make_port (state, stream, filename, MODE_OUTPUT, NULL, NULL, NULL);

    if (r_failure_p (res))
        fclose (stream);

exit:
    return res;
}

rsexp r_open_input_string (RState* state, rsexp string)
{
    rsexp    res;
    rpointer input;
    rsize    size;
    FILE*    stream;

    input  = r_cast (rpointer, r_string_to_cstr (string));
    size   = r_string_byte_count (string);
    stream = fmemopen (input, size, "r");

    if (!stream) {
        res = R_FAILURE;
        r_inherit_errno_x (state, errno);
        goto exit;
    }

    res = make_port (state, stream, "(input-string-port)",
                     MODE_INPUT | MODE_STRING_IO, r_cast (rpointer, string),
                     NULL, input_string_port_mark);

    if (r_failure_p (res))
        fclose (stream);

exit:
    return res;
}

rsexp r_open_output_string (RState* state)
{
    ROutStringCookie* cookie;
    FILE*             stream;
    rint              errnum;
    rsexp             res;

    cookie = r_new0 (state, ROutStringCookie);

    if (!cookie) {
        res = R_FAILURE;
        r_last_error (state);
        goto exit;
    }

    stream = open_memstream (&cookie->buffer, &cookie->size);

    if (!stream) {
        errnum = errno;
        r_free (state, cookie);
        r_inherit_errno_x (state, errno);
        res = R_FAILURE;
        goto exit;
    }

    res = make_port (state, stream, "(output-string-port)",
                     MODE_OUTPUT | MODE_STRING_IO | MODE_FLUSH,
                     r_cast (rpointer, cookie),
                     output_string_port_clear, NULL);

    if (r_failure_p (res))
        fclose (stream);

exit:
    return res;
}

rsexp r_get_output_string (RState* state, rsexp port)
{
    ROutStringCookie* cookie;
    RPort*            port_ptr;
    rsexp             res;

    if (!output_string_port_p (port)) {
        res = R_FAILURE;
        error_wrong_type_arg (state, "output-string-port", port);
        goto exit;
    }

    port_ptr = port_from_sexp (port);
    cookie   = r_cast (ROutStringCookie*, port_ptr->cookie);

    res = cookie->buffer
          ? r_string_new (state, cookie->buffer)
          : r_string_new (state, "");

exit:
    return res;
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

rsexp r_port_vprintf (RState*       state,
                      rsexp         port,
                      rconstcstring format,
                      va_list       args)
{
    rsexp res = R_UNSPECIFIED;

    if (vfprintf (port_to_file (port), format, args) < 0) {
        res = R_FAILURE;
        r_error (state, "vfprintf (3) failed");
        goto exit;
    }

    if (need_flush_p (port))
        if (EOF == fflush (port_to_file (port))) {
            res = R_FAILURE;
            r_error (state, "fflush (3) failed");
            goto exit;
        }

exit:
    return res;
}

rsexp r_port_printf (RState* state, rsexp port, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_port_vprintf (state, port, format, args);
    va_end (args);

    return res;
}

rcstring r_port_gets (RState* state, rsexp port, rcstring dest, rint size)
{
    rcstring res = fgets (dest, size, port_to_file (port));

    if (!res)
        r_inherit_errno_x (state, errno);

    return res;
}

rsexp r_port_puts (RState* state, rsexp port, rconstcstring str)
{
    rsexp res = R_UNSPECIFIED;

    if (EOF == fputs (str, port_to_file (port))) {
        res = R_FAILURE;
        r_error (state, "fputs (3) failed");
        goto exit;
    }

    if (need_flush_p (port))
        if (EOF == fflush (port_to_file (port))) {
            res = R_FAILURE;
            r_error (state, "fflush (3) failed");
            goto exit;
        }

exit:
    return res;
}

rsexp r_port_read_char (RState* state, rsexp port)
{
    rint ch = fgetc (port_to_file (port_from_sexp (port)));
    return (EOF == ch) ? R_EOF : r_char_to_sexp (r_cast (rchar, ch));
}

rsexp r_read_char (RState* state)
{
    return r_port_read_char (state, r_current_input_port (state));
}

rsexp r_port_write_char (RState* state, rsexp port, rchar ch)
{
    rsexp res = R_UNSPECIFIED;

    if (EOF == fputc (ch, port_to_file (port))) {
        res = R_FAILURE;
        r_error (state, "fputc (3) failed");
        goto exit;
    }

    if (need_flush_p (port))
        if (EOF == fflush (port_to_file (port))) {
            res = R_FAILURE;
            r_error (state, "fflush (3) failed");
            goto exit;
        }

exit:
    return res;
}

rsexp r_write_char (RState* state, rchar ch)
{
    return r_port_write_char (state, r_current_input_port (state), ch);
}

rsexp r_port_vformat (RState*       state,
                      rsexp         port,
                      rconstcstring format,
                      va_list       args)
{
    rconstcstring pos;

    for (pos = format; *pos; ++pos) {
        if ('~' != *pos) {
            ensure (r_port_write_char (state, port, *pos));
            continue;
        }

        switch (*++pos) {
            case '~':
                ensure (r_port_write_char (state, port, '~'));
                break;

            case '%':
                ensure (r_port_write_char (state, port, '\n'));
                break;

            case 'a':
                ensure (r_port_display (state, port, va_arg (args, rsexp)));
                break;

            case 's':
                // ensure (r_port_write (state, port, va_arg (args, rsexp)));
                r_port_write (state, port, va_arg (args, rsexp));
                break;
        }
    }

    return R_UNSPECIFIED;
}

rsexp r_port_format (RState* state, rsexp port, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_port_vformat (state, port, format, args);
    va_end (args);

    return res;
}

rsexp r_format (RState* state, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_port_vformat (state, r_current_output_port (state), format, args);
    va_end (args);

    return res;
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

rsexp r_port_write (RState* state, rsexp port, rsexp obj)
{
    RTypeInfo* type_info = r_type_info (state, obj);

    return type_info
           ? type_info->ops.write (state, port, obj)
           : R_UNSPECIFIED;
}

rsexp r_port_display (RState* state, rsexp port, rsexp obj)
{
    RTypeInfo* type_info = r_type_info (state, obj);

    return type_info
           ? type_info->ops.display (state, port, obj)
           : R_UNSPECIFIED;
}

rsexp r_write (RState* state, rsexp obj)
{
    return r_port_write (state, r_current_output_port (state), obj);
}

rsexp r_display (RState* state, rsexp obj)
{
    return r_port_display (state, r_current_output_port (state), obj);
}
