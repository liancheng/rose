#include "rose/rose.h"

#include <libgen.h>
#include <stdio.h>
#include <unistd.h>

int main (int argc, char* argv[])
{
    RState* r;
    rsexp port;
    rsexp result;
    int exit_code;

    r = r_state_open ();

    if (!r) {
        fprintf (stderr, "ROSE interpreter initialization failed.\n");
        exit_code = EXIT_FAILURE;
        goto exit;
    }

    chdir (dirname (argv [0]));
    chdir ("../");

    r_eval_from_file (r, "src/lib/init.scm");

    port = (argc > 1)
         ? r_open_input_file (r, argv [1])
         : r_current_input_port (r);

    result = r_eval_from_port (r, port);

    if (r_failure_p (result)) {
        r_format (r, "~s~%", r_last_error (r));
        exit_code = EXIT_FAILURE;
        goto clean;
    }

    exit_code = EXIT_SUCCESS;

clean:
    r_state_free (r);

exit:
    return exit_code;
}
