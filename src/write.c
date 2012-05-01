#include "rose/error.h"
#include "rose/pair.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"
#include "rose/write.h"

#include <assert.h>

void sexp_write_cdr(FILE* output, rsexp sexp, RContext* context)
{
    if (SEXP_PAIR_P(sexp)) {
        fprintf(output, " ");
        sexp_write_datum(output, sexp_car(sexp), context);
        sexp_write_cdr(output, sexp_cdr(sexp), context);
    }
    else if (!SEXP_NULL_P(sexp)) {
        fprintf(output, " . ");
        sexp_write_datum(output, sexp, context);
    }
}

void sexp_write_pair(FILE* output, rsexp sexp, RContext* context)
{
    assert(SEXP_PAIR_P(sexp));

    fprintf(output, "(");
    sexp_write_datum(output, sexp_car(sexp), context);
    sexp_write_cdr(output, sexp_cdr(sexp), context);
    fprintf(output, ")");
}

void sexp_write_vector(FILE* output, rsexp sexp, RContext* context)
{
    assert(SEXP_VECTOR_P(sexp));

    fprintf(output, "#(");

    rsize length = sexp_vector_length(sexp);
    if (length) {
        sexp_write_datum(output, sexp_vector_ref(sexp, 0), context);

        for (rsize i = 1; i < length; ++i) {
            fprintf(output, " ");
            sexp_write_datum(output, sexp_vector_ref(sexp, i), context);
        }
    }

    fprintf(output, ")");
}

void sexp_write_datum(FILE* output, rsexp sexp, RContext* context)
{
    if (SEXP_TRUE == sexp) {
        fprintf(output, "#t");
    }
    else if (SEXP_FALSE == sexp) {
        fprintf(output, "#f");
    }
    else if (SEXP_SYMBOL_P(sexp)) {
        fprintf(output, "%s", r_sexp_to_symbol(sexp, context));
    }
    else if (SEXP_PAIR_P(sexp)) {
        sexp_write_pair(output, sexp, context);
    }
    else if (SEXP_NULL_P(sexp)) {
        fprintf(output, "()");
    }
    else if (SEXP_STRING_P(sexp)) {
        fprintf(output, "\"%s\"", ((RBoxed*)sexp)->as.string.data);
    }
    else if (SEXP_VECTOR_P(sexp)) {
        sexp_write_vector(output, sexp, context);
    }
}
