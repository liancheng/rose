#include "sexp.h"
#include "lexer/r5rs_lexer"

#include <argtable2.h>
#include <readline/history.h>
#include <readline/readline.h>

#include <cstdio>
#include <cstdlib>
#include <string>

void usage(char const* program, void* args[])
{
    printf("Usage: %s", program);
    arg_print_syntax(stdout, args, "\n\n");
    arg_print_glossary(stdout, args, "  %-25s %s\n");
}

int main(int argc, char* argv[])
{
    const char* program = "sexp";

    void* args[] = {
        arg_file0("i", "input", "FILE", "Read input from FILE"),
        arg_lit0 ("h", "help",          "Print this help message and exit"),
        arg_end(10)
    };

    struct arg_file* input = (struct arg_file*)args[0];
    struct arg_lit*  help  = (struct arg_lit* )args[1];
    struct arg_end*  end   = (struct arg_end* )args[2];

    int arg_errors = arg_parse(argc, argv, args);

    if (1 == argc || help->count > 0) {
        usage(program, args);
        exit(EXIT_FAILURE);
    }

    if (arg_errors > 0) {
        arg_print_errors(stdout, end, program);
        printf("Try `%s --help' for more information.\n", program);
        exit(EXIT_FAILURE);
    }

    if (0 == input->count) {
        fprintf(stderr, "no input file.\n");
        exit(EXIT_FAILURE);
    }

    std::string input_file = input->filename[0];

    quex::r5rs_lexer qlex(input_file.c_str());

    for (quex::Token* token = qlex.token_p(); ;) {
        int token_id = qlex.receive();

        printf("[%d,%d] %s <%s>\n",
               token->line_number(),
               token->column_number(),
               token->type_id_name().c_str(),
               token->text.c_str());

        if (TKN_TERMINATION == token_id || TKN_FAIL == token_id)
            break;
    }

    return EXIT_SUCCESS;
}
