#include "detail/sexp.h"

rbool r_eq_p (RState* state, rsexp lhs, rsexp rhs)
{
    return lhs == rhs;
}

rbool r_eqv_p (RState* state, rsexp lhs, rsexp rhs)
{
    if (r_type_tag (lhs) != r_type_tag (rhs))
        return FALSE;

    RTypeInfo* desc = r_describe (state, lhs);
    REqvPred   pred = desc->ops.eqv_p;

    return r_eq_p (state, lhs, rhs)
           ? TRUE
           : pred ? pred (state, lhs, rhs)
                  : FALSE;
}

rbool r_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    if (r_type_tag (lhs) != r_type_tag (rhs))
        return FALSE;

    RTypeInfo* desc = r_describe (state, lhs);
    REqualPred pred = desc->ops.equal_p;

    return r_eqv_p (state, lhs, rhs)
           ? TRUE
           : pred ? pred (state, lhs, rhs)
                  : FALSE;
}
