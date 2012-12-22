#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"

#include <stdio.h>

int main (int argc, char* argv[])
{
    RState* state;
    rsexp   reader;

    state = r_state_open ();

    if (!state) {
        fprintf (stderr, "ROSE interpreter initialization failed.\n");

        return EXIT_FAILURE;
    }

    if (argc > 1)
        r_set_current_input_port_x
            (state, r_open_input_file (state, argv [1]));

    reader = r_reader_new (state, r_current_input_port (state));

    if (r_failure_p (reader)) {
        fprintf (stderr, "reader initialization failed.\n");
        r_state_free (state);

        return EXIT_FAILURE;
    }

    while (TRUE) {
        rsexp datum = r_read (reader);

        if (r_failure_p (datum)) {
            r_format (state, "~a~%", r_last_error (state));
            break;
        }

        if (r_eof_object_p (datum))
            break;

        r_format (state, "~s~%", datum);
    }

    r_state_free (state);

    return EXIT_SUCCESS;
}
