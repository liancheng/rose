#include "r5rs_lexer.h"
#include "sexp.h"

#include <argtable2.h>
#include <stdio.h>
#include <stdlib.h>

typedef quex_Token token;
typedef quex_r5rs_lexer lexer;
typedef QUEX_TYPE_TOKEN_ID token_id;

lexer* get_lexer()
{
    static lexer qlex;
    return &qlex;
}

void usage(char const* program, void* args[])
{
    printf("Usage: %s", program);
    arg_print_syntax(stdout, args, "\n\n");
    arg_print_glossary(stdout, args, "  %-20s %s\n");
}

token* read_token(FILE* file)
{
    lexer* qlex = get_lexer();

    token_id id = QUEX_NAME(receive)(qlex);

    // Try to reload the analyzer with more input.
    if (TKN_TERMINATION == id) {
        QUEX_NAME(buffer_fill_region_prepare)(qlex);

        // Read input from stream into analyzer buffer.
        char* begin = (char*)QUEX_NAME(buffer_fill_region_begin)(qlex);
        int   size  = QUEX_NAME(buffer_fill_region_size)(qlex);
        char* line  = fgets(begin, size, file);

        // EOF, no more input.
        if (!line) {
            QUEX_NAME(buffer_fill_region_finish)(qlex, 0);
            return NULL;
        }

        QUEX_NAME(buffer_fill_region_finish)(qlex, strlen(line));

        // Discard the last TKN_TERMINATION token,
        // get next available token. 
        QUEX_NAME(receive)(qlex);
    }

    return QUEX_NAME(token_p)(qlex);
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

    // Initialize the Quex lexer, which is a global object.
    QUEX_NAME(construct_memory)(get_lexer(), NULL, 0, NULL, NULL, false);

    token* t = NULL;
    while (t = read_token(file)) {
        static const int size = 256;
        char buffer[size];

        printf("[%d,%d] %s <%s>\n",
               t->_line_n,
               t->_column_n,
               QUEX_NAME_TOKEN(map_id_to_name)(t->_id),
               QUEX_NAME_TOKEN(pretty_char_text)(t, buffer, size));
    }

    if (file != stdin) {
        fclose(file);
    }

    QUEX_NAME(destruct)(get_lexer());

    return EXIT_SUCCESS;
}
