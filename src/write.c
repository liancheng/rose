#include "rose/error.h"
#include "rose/pair.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"
#include "rose/write.h"

#include <assert.h>

void r_write_cdr(FILE* output, rsexp sexp, RContext* context)
{
    if (SEXP_PAIR_P(sexp)) {
        fprintf(output, " ");
        r_write_datum(output, r_car(sexp), context);
        r_write_cdr(output, r_cdr(sexp), context);
    }
    else if (!SEXP_NULL_P(sexp)) {
        fprintf(output, " . ");
        r_write_datum(output, sexp, context);
    }
}

void r_write_pair(FILE* output, rsexp sexp, RContext* context)
{
    assert(SEXP_PAIR_P(sexp));

    fprintf(output, "(");
    r_write_datum(output, r_car(sexp), context);
    r_write_cdr(output, r_cdr(sexp), context);
    fprintf(output, ")");
}

void r_write_vector(FILE* output, rsexp sexp, RContext* context)
{
    rsize i;
    rsize length;

    assert(SEXP_VECTOR_P(sexp));

    fprintf(output, "#(");

    length = r_vector_length(sexp);

    if (length) {
        r_write_datum(output, r_vector_ref(sexp, 0), context);

        for (i = 1; i < length; ++i) {
            fprintf(output, " ");
            r_write_datum(output, r_vector_ref(sexp, i), context);
        }
    }

    fprintf(output, ")");
}

void r_write_datum(FILE* output, rsexp sexp, RContext* context)
{
    if (SEXP_TRUE == sexp) {
        fprintf(output, "#t");
    }
    else if (SEXP_FALSE == sexp) {
        fprintf(output, "#f");
    }
    else if (SEXP_SYMBOL_P(sexp)) {
        fprintf(output, "%s", r_symbol_name(sexp, context));
    }
    else if (SEXP_PAIR_P(sexp)) {
        r_write_pair(output, sexp, context);
    }
    else if (SEXP_NULL_P(sexp)) {
        fprintf(output, "()");
    }
    else if (SEXP_STRING_P(sexp)) {
        fprintf(output, "\"%s\"", SEXP_AS(sexp, string).data);
    }
    else if (SEXP_VECTOR_P(sexp)) {
        r_write_vector(output, sexp, context);
    }
}
