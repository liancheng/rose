#include "rose/rose.h"

#include <stdio.h>

int main (int argc, char* argv[])
{
    RState* state;
    rsexp   code;
    rsexp   result;
    rint    exit_code;

    exit_code = EXIT_FAILURE;
    state = r_state_open ();

    if (!state) {
        fprintf (stderr, "ROSE interpreter initialization failed.\n");
        goto exit;
    }

    if (argc > 1) {
        rsexp source = r_open_input_file (state, argv [1]);
        r_set_current_input_port_x (state, source);
    }

    code = r_compile_from_port (state, r_current_input_port (state));

    if (r_failure_p (code)) {
        rsexp error = r_last_error (state);
        rsexp error_code = r_car (r_error_object_irritants (error));
        rsexp message = r_error_object_message (error);
        r_format (state, "error (~a): ~a~%", error_code, message);
        goto clean;
    }

    r_format (state, "compiled code: ~s~%", code);

    result = r_run (state, code);
    r_format (state, "result: ~s~%", result);

    if (r_failure_p (result)) {
        r_format (state, "runtime error: ~a~%", r_last_error (state));
        goto clean;
    }

    exit_code = EXIT_SUCCESS;

clean:
    r_state_free (state);

exit:
    return exit_code;
}
