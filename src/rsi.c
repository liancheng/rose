#include "scanner.h"

#include "rose/env.h"
#include "rose/pair.h"
#include "rose/read.h"
#include "rose/sexp.h"
#include "rose/symbol.h"
#include "rose/write.h"

#include <gc/gc.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    FILE* in;
    rsexp context;

    in = (argc > 1) ? fopen(argv[1], "r") : stdin;

    GC_INIT();
    context = r_context_new();

    for (r_scanner_init(in, context); ;) {
        rsexp res = r_read(in, context);

        if (r_eof_p(res))
            break;

        if (r_unspecified_p(res)) {
            r_scanner_consume_token(in, context);
            printf("error\n");
        }

        r_write(stdout, res, context);
        printf("\n");
    }

    if (in != stdin)
        fclose(in);

    return EXIT_SUCCESS;
}
