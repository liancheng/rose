#include "detail/number.h"
#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/bytevector.h"
#include "rose/string.h"
#include "rose/vector.h"

rbool r_eqv_p (rsexp lhs, rsexp rhs)
{
    return (lhs == rhs)
        || r_number_eqv_p (lhs, rhs);
}

rbool r_eq_p (rsexp lhs, rsexp rhs)
{
    return lhs == rhs;
}

rbool r_equal_p (rsexp lhs, rsexp rhs)
{
    return r_pair_equal_p (lhs, rhs)
        || r_vector_equal_p (lhs, rhs)
        || r_bytevector_equal_p (lhs, rhs)
        || r_string_equal_p (lhs, rhs)
        || r_eqv_p (lhs, rhs);
}
