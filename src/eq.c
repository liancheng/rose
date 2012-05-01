#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/vector.h"

rboolean r_eqv_p(rsexp lhs, rsexp rhs)
{
    return lhs == rhs;
}

rboolean r_eq_p(rsexp lhs, rsexp rhs)
{
    return lhs == rhs;
}

static rboolean r_pair_equal_p(rsexp lhs, rsexp rhs)
{
    return (SEXP_PAIR_P(lhs) && SEXP_PAIR_P(rhs))
           ? (r_equal_p(sexp_car(lhs), sexp_car(rhs)) &&
              r_equal_p(sexp_cdr(lhs), sexp_cdr(rhs)))
           : FALSE;
}

static rboolean r_vector_equal_p(rsexp lhs, rsexp rhs)
{
    rsize size;
    rsize k;

    if (!SEXP_VECTOR_P(rhs))
        return FALSE;

    size = SEXP_AS(lhs, vector).size;

    if (SEXP_AS(rhs, vector).size != size)
        return FALSE;

    for (k = 0; k < size; ++k) {
        rsexp* lhs_data = SEXP_AS(lhs, vector).data;
        rsexp* rhs_data = SEXP_AS(rhs, vector).data;

        if (!r_equal_p(lhs_data[k], rhs_data[k]))
            return FALSE;
    }

    return TRUE;
}

rboolean r_equal_p(rsexp lhs, rsexp rhs)
{
    if (SEXP_PAIR_P(lhs))
        return r_pair_equal_p(lhs, rhs);

    if (SEXP_VECTOR_P(lhs))
        return r_vector_equal_p(lhs, rhs);

    return r_eqv_p(lhs, rhs);
}

rsexp sexp_eq_p(rsexp lhs, rsexp rhs)
{
    return r_eq_p(lhs, rhs) ? SEXP_TRUE : SEXP_FALSE;
}

rsexp sexp_eqv_p(rsexp lhs, rsexp rhs)
{
    return r_eqv_p(lhs, rhs) ? SEXP_TRUE : SEXP_FALSE;
}

rsexp sexp_equal_p(rsexp lhs, rsexp rhs)
{
    return r_equal_p(lhs, rhs) ? SEXP_TRUE : SEXP_FALSE;
}
