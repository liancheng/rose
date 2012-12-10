#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"

#include <gc/gc.h>
#include <stdio.h>

int main (int argc, char* argv[])
{
    RState* state;

    GC_INIT ();

    state = r_state_open ();

    if (!state) {
        fprintf (stderr, "ROSE interpreter initialization failed.\n");
        return EXIT_FAILURE;
    }

    if (argc > 1)
        r_set_current_input_port_x
            (state, r_open_input_file (state, argv [1]));

    while (1) {
        rsexp datum = r_read (state);

        if (r_error_p (datum)) {
            r_format (state, "~a~%", datum);
            break;
        }

        if (r_eof_object_p (datum))
            break;

        r_format (state, "~s~%", datum);
    }

    return EXIT_SUCCESS;
}
