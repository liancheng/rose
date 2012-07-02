#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/writer.h"

#include <gc/gc.h>

void display_syntax_error (rsexp error, RContext* context)
{
    rsexp irritants = r_error_get_irritants (error);

    r_format (r_current_input_port (context),
              "~a:~a:~a: ~a~%",
              r_list_ref (irritants, 0),
              r_list_ref (irritants, 1),
              r_list_ref (irritants, 2),
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
    reader = r_port_reader (input, context);

    while (1) {
        rsexp datum = r_read (reader);
        rsexp error = r_reader_last_error (reader);

        if (!r_undefined_p (error)) {
            display_syntax_error (error, context);
            break;
        }

        if (r_eof_object_p (datum))
            break;

        r_format (output, "~s~%", datum);
    }

    return EXIT_SUCCESS;
}
