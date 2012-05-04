#include "rose/env.h"
#include "rose/pair.h"
#include "rose/read.h"
#include "rose/scanner.h"
#include "rose/sexp.h"
#include "rose/symbol.h"
#include "rose/write.h"

#include <gc/gc.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    FILE* in;
    RContext* context;

    in = (argc > 1) ? fopen(argv[1], "r") : stdin;

    GC_INIT();
    context = r_context_new();

    for (r_scanner_init(in, context); ;) {
        rsexp res = r_read_datum(in, context);

        if (SEXP_EOF_P(res))
            break;

        if (SEXP_UNSPECIFIED_P(res)) {
            r_scanner_consume_token(in, context);
            printf("error\n");
        }

        r_write_datum(stdout, res, context);
        printf("\n");
    }

    if (in != stdin)
        fclose(in);

    r_context_free(context);

    return EXIT_SUCCESS;
}
