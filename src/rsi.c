#include "scanner.h"

#include "rose/error.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/read.h"
#include "rose/string.h"
#include "rose/write.h"
#include "rose.h"

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

int rose_yyparse (RReaderState* state);

int main (int argc, char* argv[])
{
    rsexp context;
    RReaderState* state;

    GC_INIT ();

    context = r_context_new ();
    set_input_port (argc, argv, context);
    set_output_port (argc, argv, context);

    state = r_reader_new (context);
    state->tree = R_SEXP_NULL;

    rose_yyparse (state);

    r_write (r_current_output_port (context), state->tree, context);
    r_newline (r_current_output_port (context));

    return EXIT_SUCCESS;
}
