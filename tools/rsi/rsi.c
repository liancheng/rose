#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"

#include <gc/gc.h>
#include <stdio.h>

void display_syntax_error (RState* state, rsexp error)
{
    rsexp irritants = r_error_get_irritants (error);

    r_format (state,
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

    state = r_state_open ();

    if (!state) {
        fprintf (stderr, "Rose initialization failed.\n");
        exit (EXIT_FAILURE);
    }

    if (argc > 1)
        r_set_current_input_port_x
            (state, r_open_input_file (state, argv [1]));

    reader = r_port_reader (state, r_current_input_port (state));

    while (1) {
        rsexp datum = r_read (state, reader);
        rsexp error = r_reader_last_error (state, reader);

        if (r_eof_object_p (datum))
            break;

        if (!r_undefined_p (error)) {
            display_syntax_error (state, error);
            r_reader_clear_error (state, reader);
        }

        r_format (state, "~s~%", datum);
    }

    r_reader_free (state, reader);
    r_state_free (state);

    return EXIT_SUCCESS;
}
