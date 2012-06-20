#include "detail/context.h"
#include "detail/number.h"
#include "rose/port.h"

#include <gc/gc.h>
#include <string.h>

static void r_fixnum_write (rsexp port, rsexp obj)
{
    r_port_puts (port, "#<fixnum>");
}

static void r_flonum_write (rsexp port, rsexp obj)
{
    r_port_puts (port, "#<flonum>");
}

static char r_number_next_char (RNumberReader* reader)
{
    char ch = '\0';

    if (reader->pos < reader->end) {
        ch = *(reader->pos);
        ++(reader->pos);
    }

    return ch;
}

static void r_number_consume (RNumberReader* reader)
{
    reader->lookahead [reader->lookahead_index] = r_number_next_char (reader);
    reader->lookahead_index = (reader->lookahead_index + 1) % 2;
}

static void r_number_consume_n (RNumberReader* reader, rint n)
{
    while (n--)
        r_number_consume (reader);
}

char r_number_lookahead (RNumberReader* reader, rint k)
{
    return reader->lookahead [(reader->lookahead_index + k) % 2];
}

rboolean r_number_read_exactness (RNumberReader* reader)
{
    rboolean pass = TRUE;

    if ('#' != r_number_lookahead (reader, 0))
        return FALSE;

    switch (r_number_lookahead (reader, 1)) {
        case 'e':
            reader->exact = TRUE;
            break;

        case 'i':
            reader->exact = FALSE;
            break;

        default:
            pass = FALSE;
            break;
    }

    if (pass) {
        r_number_consume_n (reader, 2);
        reader->seen_exact = TRUE;
    }

    return pass;
}

rboolean r_number_read_radix (RNumberReader* reader)
{
    rboolean pass= TRUE;

    if ('#' != r_number_lookahead (reader, 0))
        return FALSE;

    switch (r_number_lookahead (reader, 1)) {
        case 'b':
            reader->radix = 2;
            break;

        case 'o':
            reader->radix = 8;
            break;

        case 'd':
            reader->radix = 10;
            break;

        case 'x':
            reader->radix = 16;
            break;

        default:
            pass = FALSE;
            break;
    }

    if (pass) {
        r_number_consume_n (reader, 2);
        reader->seen_radix = TRUE;
    }

    return pass;
}

rboolean r_number_read_prefix (RNumberReader* reader)
{
    if (r_number_read_radix (reader)) {
        r_number_read_exactness (reader);
        return TRUE;
    }

    if (r_number_read_exactness (reader)) {
        r_number_read_radix (reader);
        return TRUE;
    }

    return TRUE;
}

rboolean r_number_read (RNumberReader* reader)
{
    if (r_number_read_prefix (reader))
        return TRUE;

    return FALSE;
}

RNumberReader* r_number_reader_new ()
{
    RNumberReader* reader = GC_NEW (RNumberReader);

    memset (reader, 0, sizeof (RNumberReader));

    reader->exact      = TRUE;
    reader->radix      = 10;
    reader->seen_exact = FALSE;
    reader->seen_radix = FALSE;

    return reader;
}

void r_number_reader_set_input (RNumberReader* reader,
                                char const*    begin,
                                char const*    end)
{
    reader->begin = begin;
    reader->end   = end;
    reader->pos   = begin;

    r_number_consume_n (reader, 2);
}

void r_register_fixnum_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "fixnum";
    type->write_fn   = r_fixnum_write;
    type->display_fn = r_fixnum_write;

    context->tc3_types [R_FIXNUM_TAG] = type;
}

void r_register_flonum_type (RContext* context)
{
    RType* type = GC_NEW (RType);

    type->cell_size  = 0;
    type->name       = "flonum";
    type->write_fn   = r_flonum_write;
    type->display_fn = r_flonum_write;

    context->tc3_types [R_FLONUM_TAG] = type;
}

rsexp r_string_to_number (char const* text)
{
    char const*    begin  = text;
    char const*    end    = text + strlen (text);
    RNumberReader* reader = r_number_reader_new ();

    if (setjmp (reader->error_jmp)) {
        // Exception handling.
        return R_UNSPECIFIED;
    }

    r_number_reader_set_input (reader, begin, end);
    r_number_read (reader);

    return r_int_to_sexp (atoi (text));
}

rboolean r_number_eoi_p (RNumberReader* reader)
{
    return (reader->pos == reader->end) &&
           ('\0' == r_number_lookahead (reader, 0));
}
