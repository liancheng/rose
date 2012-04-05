#include "sexp.h"
#include "lexer/r5rs_lexer"

#include <argtable2.h>
#include <readline/history.h>
#include <readline/readline.h>

#include <cstdio>
#include <cstdlib>

void usage(char const* program, void* args[])
{
    printf("Usage: %s", program);
    arg_print_syntax(stdout, args, "\n\n");
    arg_print_glossary(stdout, args, "  %-25s %s\n");
}

class tokenizer {
private:
    static const int buffer_size = 256;

public:
    tokenizer(FILE* source) :
        source(source),
        qlex((QUEX_TYPE_CHARACTER*)NULL, 0)
    {}

    quex::Token* next() {
        QUEX_TYPE_TOKEN_ID token_id = qlex.receive();

        // Try to reload the analyzer with more input.
        if (TKN_TERMINATION == token_id) {
            qlex.buffer_fill_region_prepare();

            // Read input from stream into analyzer buffer.
            char* line = fgets((char*)qlex.buffer_fill_region_begin(),
                               qlex.buffer_fill_region_size(),
                               source);

            // EOF encountered.
            if (!line) {
                qlex.buffer_fill_region_finish(0);
                return NULL;
            }

            int size = strlen(line);
            qlex.buffer_fill_region_finish(size);
        }

        return qlex.token_p();
    }

private:
    FILE* source;
    quex::r5rs_lexer qlex;
};

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

    FILE* file = NULL;

    if (input->count > 0) {
        file = fopen(input->filename[0], "r");
        if (!file) {
            fprintf(stderr, "cannot open file %s\n", input->filename[0]);
            exit(EXIT_FAILURE);
        }
    }
    else {
        file = stdin;
    }

    tokenizer t(file);

    for (quex::Token* token = t.next(); token; token = t.next()) {
        printf("[%d,%d] %s <%s>\n",
               token->line_number(),
               token->column_number(),
               token->type_id_name().c_str(),
               token->text.c_str());
    }

    if (file != stdin) {
        fclose(file);
    }

    return EXIT_SUCCESS;
}
