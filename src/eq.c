#include "detail/sexp.h"

rbool r_eq_p (RState* r, rsexp lhs, rsexp rhs)
{
    return lhs == rhs;
}

rbool r_eqv_p (RState* r, rsexp lhs, rsexp rhs)
{
    RTypeInfo* type;
    REqvPred   pred;

    if (r_type_tag (lhs) != r_type_tag (rhs))
        return FALSE;

    type = r_type_info (r, lhs);
    pred = type->ops.eqv_p;

    return r_eq_p (r, lhs, rhs)
           ? TRUE
           : (pred ? pred (r, lhs, rhs)
                   : FALSE);
}

rbool r_equal_p (RState* r, rsexp lhs, rsexp rhs)
{
    RTypeInfo* type;
    REqualPred pred;

    if (r_type_tag (lhs) != r_type_tag (rhs))
        return FALSE;

    type = r_type_info (r, lhs);
    pred = type->ops.equal_p;

    return r_eqv_p (r, lhs, rhs)
           ? TRUE
           : (pred ? pred (r, lhs, rhs)
                   : FALSE);
}
