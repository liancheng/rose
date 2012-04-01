#include "sexp.h"
#include "lexer/r5rs_lexer"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>

void handle_token(quex::Token* token)
{
    printf("[%d,%d] %s <%s>\n",
           token->line_number(),
           token->column_number(),
           token->type_id_name().c_str(),
           token->text.c_str());
}

int main(int argc, char* argv[])
{
    quex::r5rs_lexer qlex((QUEX_TYPE_CHARACTER*)NULL, 0);
    quex::Token* token = NULL;

    for (char* line; (line = readline("~> ")); free(line)) {
        add_history(line);
        write_history(0);

        std::string str(line);
        str.push_back('\n');

        qlex.buffer_fill_region_prepare();
        std::copy(str.begin(), str.end(),
                  (QUEX_TYPE_CHARACTER*)qlex.buffer_fill_region_begin());

        qlex.buffer_fill_region_finish(str.length());

        do {
            (void)qlex.receive(&token);
            handle_token(token);
        }
        while (TKN_TERMINATION != token->type_id() &&
               TKN_FAIL != token->type_id());
    }

    return 0;
}
