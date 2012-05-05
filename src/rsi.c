#include "rose/env.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/read.h"
#include "rose/sexp.h"
#include "rose/symbol.h"
#include "rose/write.h"

#include <stdio.h>
#include <stdlib.h>

void set_io_port(int argc, char* argv[], rsexp context)
{
    rsexp input;
    rsexp output;

    if (argc > 1)
        input = r_file_input_port_new(fopen(argv[1], "r"), argv[1], TRUE);
    else
        input = r_file_input_port_new(stdin, "(standard-input)", FALSE);

    output = r_file_output_port_new(stdout, "(standard-output)", FALSE);

    r_set_current_input_port(input, context);
    r_set_current_output_port(output, context);
}

int main(int argc, char* argv[])
{
    rsexp context;
    rsexp input;
    rsexp output;

    GC_INIT();
    context = r_context_new();

    set_io_port(argc, argv, context);

    input = r_get_current_input_port(context);
    output = r_get_current_output_port(context);

    while (TRUE) {
        rsexp res = r_read(input, context);

        if (r_eof_p(res) || r_unspecified_p(res))
            break;

        r_write(output, res, context);
        printf("\n");
    }

    return EXIT_SUCCESS;
}
