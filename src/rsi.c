#include "rose/env.h"
#include "rose/pair.h"
#include "rose/read.h"
#include "rose/scanner.h"
#include "rose/sexp.h"
#include "rose/symbol.h"
#include "rose/write.h"

#include <argtable2.h>
#include <gc/gc.h>
#include <stdio.h>
#include <stdlib.h>

void usage(char const* program, void* args[])
{
    printf("Usage: %s", program);
    arg_print_syntax(stdout, args, "\n\n");
    arg_print_glossary(stdout, args, "  %-20s %s\n");
}

int main(int argc, char* argv[])
{
    const char* program = "sexp";

    void* args[] = {
        arg_file0("i", "input", "FILE", "Read input from FILE."),
        arg_rem  (NULL,                 "Default to STDIN."),
        arg_lit0 ("h", "help",          "Print this help message and exit."),
        arg_end  (10)
    };

    struct arg_file* input = (struct arg_file*)args[0];
    struct arg_lit*  help  = (struct arg_lit* )args[2];
    struct arg_end*  end   = (struct arg_end* )args[3];

    int arg_errors = arg_parse(argc, argv, args);

    if (help->count > 0) {
        usage(program, args);
        exit(EXIT_FAILURE);
    }

    if (arg_errors > 0) {
        arg_print_errors(stdout, end, program);
        printf("Try `%s --help' for more information.\n", program);
        exit(EXIT_FAILURE);
    }

    FILE* in = stdin;

    if (input->count > 0) {
        in = fopen(input->filename[0], "r");
        if (!in) {
            fprintf(stderr, "cannot open file %s\n", input->filename[0]);
            exit(EXIT_FAILURE);
        }
    }

    GC_INIT();
    RContext* context = r_context_new();

    for (r_scanner_init(in, context); ;) {
        rsexp res = sexp_read_datum(in, context);

        if (SEXP_EOF_P(res))
            break;

        if (SEXP_UNSPECIFIED_P(res)) {
            r_scanner_consume_token(in, context);
            printf("error\n");
        }

        sexp_write_datum(stdout, res, context);
        printf("\n");
    }


    if (in != stdin) {
        fclose(in);
    }

    r_context_free(context);

    return EXIT_SUCCESS;
}
