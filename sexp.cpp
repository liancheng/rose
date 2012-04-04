#include "sexp.h"
#include "lexer/r5rs_lexer"

#include <getopt.h>
#include <readline/history.h>
#include <readline/readline.h>

#include <cstdio>
#include <cstdlib>
#include <string>

struct config {
    std::string input_file;
}
config;

void usage()
{
    printf("Usage: sexp [options]\n"
           "\n"
           "OPTIONS:\n"
           "  -h, --help           Show this help message.\n"
           "  -i, --input=FILE     Read input from FILE.\n");

    exit(EXIT_SUCCESS);
}

void handle_argv(int argc, char* argv[])
{
    static struct option long_options[] = {
        {"input", required_argument, NULL, 'i'},
    };

    while (true) {
        int option_index = 0;
        int opt = getopt_long(argc,
                              argv,
                              "hi:",
                              long_options,
                              &option_index);

        if (-1 == opt) {
            break;
        }

        switch (opt) {
            case 'h':
                usage();
                break;

            case 'i':
                config.input_file = optarg;
                break;

            default:
                break;
        }
    }

    if (config.input_file.empty()) {
        fprintf(stderr, "no input file.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    handle_argv(argc, argv);

    quex::r5rs_lexer qlex(config.input_file.c_str());
    quex::Token* token = NULL;

    do {
        (void)qlex.receive(&token);

        printf("[%d,%d] %s <%s>\n",
               token->line_number(),
               token->column_number(),
               token->type_id_name().c_str(),
               token->text.c_str());
    }
    while (TKN_TERMINATION != token->type_id() &&
           TKN_FAIL != token->type_id());

    return EXIT_SUCCESS;
}
