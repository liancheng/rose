#include "rose/compile.h"
#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/vm.h"

#include <stdio.h>

int main (int argc, char* argv[])
{
    RState* state;
    rsexp   source;
    rsexp   reader;
    rsexp   program;
    rsexp   datum;
    rsexp   code;
    rint    exit_code;

    state = r_state_open ();

    if (!state) {
        fprintf (stderr, "ROSE interpreter initialization failed.\n");
        exit_code = EXIT_FAILURE;
        goto exit;
    }

    if (argc > 1) {
        source = r_open_input_file (state, argv [1]);
        r_set_current_input_port_x (state, source);
    }

    reader = r_reader_new (state, r_current_input_port (state));

    if (r_failure_p (reader)) {
        fprintf (stderr, "reader initialization failed.\n");
        exit_code = EXIT_FAILURE;
        goto clean;
    }

    program = R_NULL;
    datum   = R_UNDEFINED;

    while (TRUE) {
        datum = r_read (reader);

        if (r_failure_p (datum)) {
            r_format (state, "~a~%", r_last_error (state));
            break;
        }

        if (r_eof_object_p (datum))
            break;

        program = r_cons (state, datum, program);
    }

    code = r_compile (state, program);

    if (r_failure_p (code)) {
        r_format (state,
                  "compilation error: ~a~%",
                  r_error_object_message (r_last_error (state)));
        goto clean;
    }

    r_format (state, "result: ~s~%", r_run (state, code));

    exit_code = EXIT_SUCCESS;

clean:
    r_state_free (state);

exit:
    return exit_code;
}
