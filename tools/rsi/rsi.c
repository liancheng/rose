#include "rose/rose.h"

#include <stdio.h>

int main (int argc, char* argv[])
{
    RState* r;
    rsexp   code;
    rsexp   result;
    rint    exit_code;

    exit_code = EXIT_FAILURE;
    r = r_state_open ();

    if (!r) {
        fprintf (stderr, "ROSE interpreter initialization failed.\n");
        goto exit;
    }

    if (argc > 1) {
        rsexp source = r_open_input_file (r, argv [1]);
        r_set_current_input_port_x (r, source);
    }

    code = r_compile_from_port (r, r_current_input_port (r));

    if (r_failure_p (code)) {
        rsexp error = r_last_error (r);
        rsexp error_code = r_car (r_error_object_irritants (error));
        rsexp message = r_error_object_message (error);
        r_format (r, "error (~a): ~a~%", error_code, message);
        goto clean;
    }

    r_format (r, "compiled code: ~s~%", code);

    result = r_run (r, code);
    r_format (r, "result: ~s~%", result);

    if (r_failure_p (result)) {
        r_format (r, "runtime error: ~a~%", r_last_error (r));
        goto clean;
    }

    exit_code = EXIT_SUCCESS;

clean:
    r_state_free (r);

exit:
    return exit_code;
}
