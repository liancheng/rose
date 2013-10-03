#include "detail/io.h"
#include "detail/state.h"
#include "rose/error.h"
#include "rose/gc.h"
#include "rose/number.h"
#include "rose/pair.h"
#include "rose/string.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct RPort RPort;

struct RPort {
    R_OBJECT_HEADER

    RState*          r;
    FILE*            stream;
    rsexp            name;
    RPortMode        mode;

    rpointer         cookie;
    RPortClearCookie cookie_clear;
    RPortMarkCookie  cookie_mark;
};

#define port_from_sexp(obj)     (r_cast (RPort*, (obj)))
#define port_to_sexp(port)      (r_cast (rsexp, (port)))

#define mode_set_p(port, m)     ((port)->mode & (m))
#define set_mode_x(port, m)     ((port)->mode |= (m))
#define clear_mode_x(port, m)   ((port)->mode &= ~(m))

static rsexp make_port (RState* r,
                        FILE* stream,
                        rconstcstring name,
                        RPortMode mode,
                        rpointer cookie,
                        RPortClearCookie clear_fn,
                        RPortMarkCookie mark_fn)
{
    rsexp name_str;
    rsexp res;
    RPort* port;

    r_gc_scope_open (r);

    name_str = r_string_new (r, name);

    if (r_failure_p (name_str)) {
        res = R_FAILURE;
        goto exit;
    }

    port = r_object_new (r, RPort, R_TAG_PORT);

    if (!port) {
        res = R_FAILURE;
        goto exit;
    }

    port->name         = name_str;
    port->r            = r;
    port->stream       = stream;
    port->mode         = mode;
    port->cookie       = cookie;
    port->cookie_clear = clear_fn;
    port->cookie_mark  = mark_fn;

    res = port_to_sexp (port);

exit:
    r_gc_scope_close_and_protect (r, res);

    return res;
}

static rsexp port_write (RState* r, rsexp port, rsexp obj)
{
    return r_port_format (r, port,
            "#<port ~a>", port_from_sexp (obj)->name);
}

static void port_finalize (RState* r, RObject* obj)
{
    RPort* port = r_cast (RPort*, obj);

    if (port->stream)
        r_close_port (port_to_sexp (port));
}

static void input_string_port_mark (RState* r, rpointer cookie)
{
    if (cookie)
        r_gc_mark (r, r_cast (rsexp, cookie));
}

typedef struct {
    rcstring buffer;
    rsize size;
}
OStrCookie;

static void output_string_port_clear (RState* r, rpointer cookie)
{
    OStrCookie* c;

    if (cookie) {
        c = r_cast (OStrCookie*, cookie);
        free (c->buffer);
        r_free (r, c);
    }
}

static rbool output_string_port_p (rsexp port)
{
    return mode_set_p (port_from_sexp (port), MODE_STRING_IO | MODE_OUTPUT);
}

static void port_mark (RState* r, rsexp obj)
{
    RPort* port = port_from_sexp (obj);

    r_gc_mark (r, port->name);

    if (port->cookie_mark)
        port->cookie_mark (r, port->cookie);
}

FILE* port_to_stream (rsexp port)
{
    return port_from_sexp (port)->stream;
}

rsexp r_open_input_file (RState* r, rconstcstring filename)
{
    FILE* stream;
    rsexp res;

    stream = fopen (filename, "r");

    if (!stream) {
        res = R_FAILURE;
        r_inherit_errno_x (r, errno);
        goto exit;
    }

    res = make_port (r, stream, filename, MODE_INPUT, NULL, NULL, NULL);

    if (r_failure_p (res))
        fclose (stream);

exit:
    return res;
}

rsexp r_open_output_file (RState* r, rconstcstring filename)
{
    FILE* stream;
    rsexp res;

    stream = fopen (filename, "w");

    if (!stream) {
        res = R_FAILURE;
        r_inherit_errno_x (r, errno);
        goto exit;
    }

    res = make_port (r, stream, filename, MODE_OUTPUT, NULL, NULL, NULL);

    if (r_failure_p (res))
        fclose (stream);

exit:
    return res;
}

rsexp r_open_input_string (RState* r, rsexp string)
{
    rsexp res;
    rpointer input;
    rsize size;
    FILE* stream;

    input = r_cast (rpointer, r_string_to_cstr (string));
    size = r_uint_from_sexp (r_string_length_by_byte (string));

    if (size == 0)
        stream = fopen ("/dev/null", "r");
    else
        stream = fmemopen (input, size, "r");

    if (!stream) {
        res = R_FAILURE;
        r_inherit_errno_x (r, errno);
        goto exit;
    }

    res = make_port (r, stream, "(input-string-port)",
                     MODE_INPUT | MODE_STRING_IO, r_cast (rpointer, string),
                     NULL, input_string_port_mark);

    if (r_failure_p (res))
        fclose (stream);

exit:
    return res;
}

rsexp r_open_output_string (RState* r)
{
    OStrCookie* cookie;
    FILE* stream;
    rintw errnum;
    rsexp res;

    cookie = r_new0 (r, OStrCookie);

    if (!cookie) {
        res = R_FAILURE;
        r_last_error (r);
        goto exit;
    }

    stream = open_memstream (&cookie->buffer, &cookie->size);

    if (!stream) {
        errnum = errno;
        r_inherit_errno_x (r, errnum);
        res = R_FAILURE;
        goto clean;
    }

    res = make_port (r, stream, "(output-string-port)",
                     MODE_OUTPUT | MODE_STRING_IO, r_cast (rpointer, cookie),
                     output_string_port_clear, NULL);

    if (r_failure_p (res)) {
        fclose (stream);
        goto clean;
    }

    goto exit;

clean:
    r_free (r, cookie);

exit:
    return res;
}

rsexp r_get_output_string (RState* r, rsexp port)
{
    OStrCookie* cookie;
    RPort* port_ptr;
    rsexp res;

    if (!output_string_port_p (port)) {
        res = R_FAILURE;
        r_error_code (r, R_ERR_WRONG_TYPE_ARG, port);
        goto exit;
    }

    if (EOF == fflush (port_to_stream (port))) {
        res = R_FAILURE;
        r_error (r, "fflush (3) failed");
        goto exit;
    }

    port_ptr = port_from_sexp (port);
    cookie   = r_cast (OStrCookie*, port_ptr->cookie);

    res = cookie->buffer
          ? r_string_new (r, cookie->buffer)
          : r_string_new (r, "");

exit:
    return res;
}

rsexp r_stdin_port (RState* r)
{
    return make_port (r, stdin, "(standard-input)",
                      MODE_INPUT | MODE_DONT_CLOSE, NULL, NULL, NULL);
}

rsexp r_stdout_port (RState* r)
{
    return make_port (r, stdout, "(standard-output)",
                      MODE_OUTPUT | MODE_DONT_CLOSE, NULL, NULL, NULL);
}

rsexp r_stderr_port (RState* r)
{
    return make_port (r, stderr, "(standard-error)",
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

    if (mode_set_p (p, MODE_CLOSED))
        return;

    fclose (p->stream);

    if (p->cookie_clear)
        p->cookie_clear (p->r, p->cookie);

    set_mode_x (port_from_sexp (port), MODE_CLOSED);
}

rbool r_eof_p (rsexp port)
{
    return 0 != feof (port_to_stream (port));
}

rbool r_port_p (rsexp obj)
{
    return r_type_tag (obj) == R_TAG_PORT;
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

rsexp r_port_vprintf (RState* r,
                      rsexp port,
                      rconstcstring format,
                      va_list args)
{
    rsexp res = R_UNSPECIFIED;

    if (vfprintf (port_to_stream (port), format, args) < 0) {
        res = R_FAILURE;
        r_error (r, "vfprintf (3) failed");
        goto exit;
    }

exit:
    return res;
}

rsexp r_port_printf (RState* r, rsexp port, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_port_vprintf (r, port, format, args);
    va_end (args);

    return res;
}

rsexp r_printf (RState* r, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_port_vprintf (r, r_current_input_port (r), format, args);
    va_end (args);

    return res;
}

rcstring r_port_gets (RState* r, rsexp port, rcstring dest, rintw size)
{
    return fgets (dest, size, port_to_stream (port));
}

rsexp r_port_puts (RState* r, rsexp port, rconstcstring str)
{
    rsexp res = R_UNSPECIFIED;

    if (EOF == fputs (str, port_to_stream (port))) {
        res = R_FAILURE;
        r_error (r, "fputs (3) failed");
        goto exit;
    }

exit:
    return res;
}

rsexp r_port_read_char (RState* r, rsexp port)
{
    rintw ch = fgetc (port_to_stream (port));
    return (EOF == ch) ? R_EOF : r_char_to_sexp (r_cast (rchar, ch));
}

rsexp r_read_char (RState* r)
{
    return r_port_read_char (r, r_current_input_port (r));
}

rsexp r_port_write_char (RState* r, rsexp port, rchar ch)
{
    rsexp res = R_UNSPECIFIED;

    if (EOF == fputc (ch, port_to_stream (port))) {
        res = R_FAILURE;
        r_error (r, "fputc (3) failed");
        goto exit;
    }

exit:
    return res;
}

rsexp r_write_char (RState* r, rchar ch)
{
    return r_port_write_char (r, r_current_input_port (r), ch);
}

rsexp r_port_vformat (RState* r,
                      rsexp port,
                      rconstcstring format,
                      va_list args)
{
    rconstcstring pos;

    for (pos = format; *pos; ++pos) {
        if ('~' != *pos) {
            ensure (r_port_write_char (r, port, *pos));
            continue;
        }

        switch (*++pos) {
            case '~':
                ensure (r_port_write_char (r, port, '~'));
                break;

            case '%':
                ensure (r_port_write_char (r, port, '\n'));
                break;

            case 'a':
                ensure (r_port_display (r, port, va_arg (args, rsexp)));
                break;

            case 's':
                ensure (r_port_write (r, port, va_arg (args, rsexp)));
                break;
        }
    }

    return R_UNSPECIFIED;
}

rsexp r_port_format (RState* r, rsexp port, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_port_vformat (r, port, format, args);
    va_end (args);

    return res;
}

rsexp r_format (RState* r, rconstcstring format, ...)
{
    va_list args;
    rsexp   res;

    va_start (args, format);
    res = r_port_vformat (r, r_current_output_port (r), format, args);
    va_end (args);

    return res;
}

rsexp r_current_input_port (RState* r)
{
    return r->current_input_port;
}

rsexp r_current_output_port (RState* r)
{
    return r->current_output_port;
}

rsexp r_current_error_port (RState* r)
{
    return r->current_error_port;
}

void r_set_current_input_port_x (RState* r, rsexp port)
{
    r->current_input_port = port;
}

void r_set_current_output_port_x (RState* r, rsexp port)
{
    r->current_output_port = port;
}

void r_set_current_error_port_x (RState* r, rsexp port)
{
    r->current_error_port = port;
}

rsexp r_port_write (RState* r, rsexp port, rsexp obj)
{
    RTypeInfo* type = r_type_info (r, obj);

    return type->ops.write
           ? type->ops.write (r, port, obj)
           : r_port_printf (r, port, "#<%s>", type->name);
}

rsexp r_port_display (RState* r, rsexp port, rsexp obj)
{
    RTypeInfo* type = r_type_info (r, obj);

    return type->ops.display
           ? type->ops.display (r, port, obj)
           : r_port_printf (r, port, "#<%s>", type->name);
}

rsexp r_write (RState* r, rsexp obj)
{
    return r_port_write (r, r_current_output_port (r), obj);
}

rsexp r_display (RState* r, rsexp obj)
{
    return r_port_display (r, r_current_output_port (r), obj);
}

RTypeInfo port_type = {
    .size = sizeof (RPort),
    .name = "port",
    .ops = {
        .write = port_write,
        .display = port_write,
        .mark = port_mark,
        .finalize = port_finalize
    }
};
