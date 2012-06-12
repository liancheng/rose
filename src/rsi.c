#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/writer.h"

#include <gc/gc.h>

void display_syntax_error (rsexp error, RContext* context)
{
    rsexp irritants = r_error_irritants (error);
    rsexp line      = r_car (irritants);
    rsexp column    = r_cdr (irritants);
    rsexp message   = r_error_message (error);

    r_port_printf (r_current_input_port (context),
                   "%d:%d %s\n",
                   r_int_from_sexp (line),
                   r_int_from_sexp (column),
                   r_string_cstr (message));
}

int main (int argc, char* argv[])
{
    RContext* context;
    RReaderState* reader;
    rsexp input;
    rsexp output;

    GC_INIT ();

    context = r_context_new ();

    if (argc > 1) {
        rsexp port = r_open_input_file (argv [1]);
        r_set_current_input_port_x (port, context);
    }

    input  = r_current_input_port (context);
    output = r_current_output_port (context);
    reader = r_reader_from_port (input, context);

    while (1) {
        rsexp datum = r_read (reader);

        if (r_eof_object_p (datum))
            break;

        if (r_reader_error_p (reader)) {
            rsexp error = r_reader_last_error (reader);
            display_syntax_error (error, context);
            break;
        }

        r_write (output, datum, context);
        r_write_char (output, '\n');
    }

    return EXIT_SUCCESS;
}
