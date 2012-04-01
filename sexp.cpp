#include "sexp.h"
#include "lexer/r5rs_lexer"

#include <readline/history.h>
#include <readline/readline.h>

#include <stdio.h>

int main(int argc, char* argv[])
{
    quex::r5rs_lexer qlex(argv[1]);
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
