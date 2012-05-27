#include "scanner.h"

#include "rose/error.h"
#include "rose/parser.h"
#include "rose/port.h"
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
    rsexp in;
    rsexp out;
    RParserState* parser;

    GC_INIT ();

    context = r_context_new ();
    in      = set_input_port (argc, argv, context);
    out     = set_output_port (argc, argv, context);
    parser  = r_parse_port (in, context);

    if (!r_undefined_p (r_parser_error (parser))) {
        rsexp error = r_parser_last_error (parser);
        r_display (out, r_error_message (error), context);
    }
    else
        r_write (out, r_parser_result (parser), context);

    r_newline (out);
    r_close_input_port (in);
    r_close_output_port (out);

    return EXIT_SUCCESS;
}
