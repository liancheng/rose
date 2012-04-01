#include "sexp.h"
#include "lexer/r5rs_lexer"

#include <getopt.h>
#include <readline/history.h>
#include <readline/readline.h>

#include <stdio.h>
#include <string>

static struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
};

int main(int argc, char* argv[])
{
    std::string input;

    while (true) {
        int option_index = 0;
        int opt = getopt_long(argc,
                              argv,
                              "i:",
                              long_options,
                              &option_index);

        if (-1 == opt) {
            break;
        }

        switch (opt) {
            case 'i':
                input = optarg;
                break;

            default:
                break;
        }
    }

    if (input.empty()) {
        fprintf(stderr, "no input file.\n");
        return EXIT_FAILURE;
    }

    quex::r5rs_lexer qlex(input.c_str());
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
