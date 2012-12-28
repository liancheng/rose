#include "rose/rose.h"

#include <stdio.h>

int main (int argc, char* argv[])
{
    RState* state;
    rsexp   source;
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

    code = r_compile_from_port (state, r_current_input_port (state));

    if (r_failure_p (code)) {
        r_format (state,
                  "compilation error: ~a~%",
                  r_error_object_message (r_last_error (state)));
        goto clean;
    }

    r_format (state, "compiled code: ~s~%", code);
    r_format (state, "result: ~s~%", r_run (state, code));

    exit_code = EXIT_SUCCESS;

clean:
    r_state_free (state);

exit:
    return exit_code;
}
