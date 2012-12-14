#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"

#include <gc/gc.h>
#include <stdio.h>

void repl_start (RState* state)
{
    RDatumReader* reader = r_reader_new (state, r_current_input_port (state));

    if (!reader) {
        fprintf (stderr, "reader initialization failed.\n");
        r_state_free (state);
        abort ();
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

    r_reader_free (reader);
}

int main (int argc, char* argv[])
{
    RState* state;

    GC_INIT ();

    state = r_state_open ();

    if (!state) {
        fprintf (stderr, "ROSE interpreter initialization failed.\n");
        abort ();
    }

    if (argc > 1)
        r_set_current_input_port_x
            (state, r_open_input_file (state, argv [1]));

    repl_start (state);
    r_state_free (state);

    return EXIT_SUCCESS;
}
