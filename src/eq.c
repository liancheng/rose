#include "detail/number.h"
#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/bytevector.h"
#include "rose/string.h"
#include "rose/vector.h"

rbool r_eq_p (RState* state, rsexp lhs, rsexp rhs)
{
    return lhs == rhs;
}

rbool r_eqv_p (RState* state, rsexp lhs, rsexp rhs)
{
    return r_eq_p (state, lhs, rhs)
        || r_number_eqv_p (state, lhs, rhs);
}

rbool r_equal_p (RState* state, rsexp lhs, rsexp rhs)
{
    return r_eqv_p (state, lhs, rhs)
        || r_pair_equal_p (state, lhs, rhs)
        || r_vector_equal_p (state, lhs, rhs)
        || r_bytevector_equal_p (state, lhs, rhs)
        || r_string_equal_p (state, lhs, rhs);
}
