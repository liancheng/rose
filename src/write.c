#include "boxed.h"

#include "rose/pair.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"
#include "rose/write.h"

#include <assert.h>

void r_write(rsexp output, rsexp sexp, rsexp context)
{
    if (R_SEXP_TRUE == sexp) {
        r_pprintf(output, "#t");
    }
    else if (R_SEXP_FALSE == sexp) {
        r_pprintf(output, "#f");
    }
    else if (r_symbol_p(sexp)) {
        r_write_symbol(output, sexp, context);
    }
    else if (r_pair_p(sexp)) {
        r_write_pair(output, sexp, context);
    }
    else if (r_null_p(sexp)) {
        r_write_null(output, sexp, context);
    }
    else if (r_string_p(sexp)) {
        r_write_string(output, sexp, context);
    }
    else if (r_vector_p(sexp)) {
        r_write_vector(output, sexp, context);
    }
}
