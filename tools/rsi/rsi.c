#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/writer.h"

#include <gc/gc.h>

void display_syntax_error (RState* state, rsexp error)
{
    rsexp irritants = r_error_get_irritants (error);

    r_format (r_current_output_port (state),
              "~a:~a:~a: ~a~%",
              r_list_ref (irritants, 0),
              r_list_ref (irritants, 1),
              r_list_ref (irritants, 2),
              r_error_get_message (error));
}

int main (int argc, char* argv[])
{
    RState*       state;
    RDatumReader* reader;

    GC_INIT ();

    state = r_state_new ();

    if (argc > 1)
        r_set_current_input_port_x
            (state, r_open_input_file (state, argv [1]));

    reader = r_port_reader (state, r_current_input_port (state));

    while (1) {
        rsexp datum = r_read (reader);
        rsexp error = r_reader_last_error (reader);

        if (r_eof_object_p (datum))
            break;

        if (!r_undefined_p (error)) {
            display_syntax_error (state, error);
            r_reader_clear_error (reader);
        }

        r_format (r_current_output_port (state), "~s~%", datum);
    }

    return EXIT_SUCCESS;
}
