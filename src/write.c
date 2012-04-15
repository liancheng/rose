#include "rose/pair.h"
#include "rose/symbol.h"
#include "rose/write.h"

#include <assert.h>

void sexp_write_cdr(FILE* output, r_sexp sexp, r_context* context)
{
    if (sexp_pair_p(sexp)) {
        fprintf(output, " ");
        sexp_write_datum(output, sexp_car(sexp), context);
        sexp_write_cdr(output, sexp_cdr(sexp), context);
    }
    else if (!sexp_null_p(sexp)) {
        fprintf(output, " . ");
        sexp_write_datum(output, sexp, context);
    }
}

void sexp_write_pair(FILE* output, r_sexp sexp, r_context* context)
{
    assert(sexp_pair_p(sexp));
    fprintf(output, "(");
    sexp_write_datum(output, sexp_car(sexp), context);
    sexp_write_cdr(output, sexp_cdr(sexp), context);
    fprintf(output, ")");
}

void sexp_write_datum(FILE* output, r_sexp sexp, r_context* context)
{
    if (SEXP_TRUE == sexp) {
        fprintf(output, "#t");
    }
    else if (SEXP_FALSE == sexp) {
        fprintf(output, "#f");
    }
    else if (sexp_symbol_p(sexp)) {
        fprintf(output, "%s", sexp_to_symbol(sexp, context));
    }
    else if (sexp_pair_p(sexp)) {
        sexp_write_pair(output, sexp, context);
    }
}

