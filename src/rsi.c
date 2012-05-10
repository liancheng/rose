#include "scanner.h"

#include "rose/error.h"
#include "rose/port.h"
#include "rose/read.h"
#include "rose/write.h"

rsexp set_input_port (int argc, char* argv[], rsexp context)
{
    rsexp port;

    port = (argc > 1)
         ? r_open_input_file (argv[1])
         : r_stdin_port ();

    if (!r_error_p (port))
        r_set_current_input_port_x (port, context);

    return port;
}

rsexp set_output_port (int argc, char* argv[], rsexp context)
{
    rsexp port;

    port = (argc > 2)
         ? r_open_output_file (argv[2])
         : r_stdout_port ();

    if (!r_error_p (port))
        r_set_current_output_port_x (port, context);

    return port;
}

int main (int argc, char* argv[])
{
    rsexp context;
    rsexp i_port;
    rsexp o_port;

    GC_INIT ();

    context = r_context_new ();
    i_port  = set_input_port (argc, argv, context);
    o_port  = set_output_port (argc, argv, context);

    // TODO I/O port error handling

    while (TRUE) {
        rsexp res = r_read (i_port, READ_EXPECT, context);

        if (r_eof_object_p (res))
            break;

        r_display (o_port, res, context);
        r_port_puts (o_port, "\n");
    }

    r_close_input_port (i_port);
    r_close_output_port (o_port);

    return EXIT_SUCCESS;
}
