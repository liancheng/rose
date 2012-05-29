#include "scanner.h"

#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/read.h"
#include "rose/string.h"
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

void display_syntax_error (rsexp port, rsexp error)
{
    int line = r_car (r_error_irritants (error));
    int column = r_cdr (r_error_irritants (error));
    char const* message = r_string_cstr (r_error_message (error));

    r_port_printf (port, "%d:%d %s\n", line, column, message);
}

int main (int argc, char* argv[])
{
    rsexp context;
    rsexp in;
    rsexp out;
    RReaderState* reader;

    GC_INIT ();

    context = r_context_new ();
    in      = set_input_port (argc, argv, context);
    out     = set_output_port (argc, argv, context);
    reader  = r_read_from_port (in, context);

    if (!r_undefined_p (r_reader_error (reader)))
        display_syntax_error (out, r_reader_last_error (reader));
    else
        r_write (out, r_reader_result (reader), context);

    r_newline (out);
    r_close_input_port (in);
    r_close_output_port (out);

    return EXIT_SUCCESS;
}
