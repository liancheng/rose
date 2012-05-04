#include "boxed.h"

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

rboolean r_equal_p(rsexp lhs, rsexp rhs)
{
    return (r_pair_p(lhs) && r_pair_equal_p(lhs, rhs)) ||
           (r_vector_p(lhs) && r_vector_equal_p(lhs, rhs)) ||
           r_eqv_p(lhs, rhs);
}
