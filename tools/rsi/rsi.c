#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/writer.h"

#include <gc/gc.h>

void display_syntax_error (rsexp error, RContext* context)
{
    rsexp irritants = r_error_get_irritants (error);

    r_format (r_current_input_port (context),
              "~a:~a ~a~%",
              r_car (irritants),
              r_cdr (irritants),
              r_error_get_message (error));
}

int main (int argc, char* argv[])
{
    RContext* context;
    RDatumReader* reader;
    rsexp input;
    rsexp output;

    GC_INIT ();

    context = r_context_new ();

    if (argc > 1) {
        rsexp port = r_open_input_file (argv [1], context);
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

        r_format (output, "~s~%", datum);
    }

    return EXIT_SUCCESS;
}
