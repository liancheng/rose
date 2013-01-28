#include "utils.hpp"

#include "rose/bytevector.h"
#include "rose/eq.h"
#include "rose/number.h"
#include "rose/pair.h"

class test_bytevector : public fixture_base {};

TEST_F (test_bytevector, r_bytevector_new)
{
    // Empty bytevector
    {
        rsexp actual = r_bytevector_new (r, 0u, 0);
        EXPECT_FALSE (r_failure_p (actual));
    }

    // Allocation failure
    {
        allocator_disruptor ad;
        rsexp actual = r_bytevector_new (r, 0u, 0);
        EXPECT_TRUE (r_failure_p (actual));
    }

    // Non-empty bytevector
    {
        rsexp actual = r_bytevector_new (r, 1u, 1);
        EXPECT_FALSE (r_failure_p (actual));
        EXPECT_EQ (R_ONE, r_bytevector_length (actual));
    }
}

TEST_F (test_bytevector, r_equal_p)
{
    {
        rsexp lhs = r_bytevector_new (r, 0u, 0);
        rsexp rhs = r_bytevector_new (r, 0u, 0);
        EXPECT_TRUE (equal_p (lhs, rhs));
    }

    {
        rsexp lhs = r_bytevector_new (r, 0u, 0);
        rsexp rhs = R_FALSE;
        EXPECT_FALSE (equal_p (lhs, rhs));
    }

    {
        rsexp lhs = r_bytevector_new (r, 0u, 0);
        rsexp rhs = r_bytevector_new (r, 1u, 0);
        EXPECT_FALSE (equal_p (lhs, rhs));
    }

    {
        rsexp lhs = r_bytevector_new (r, 1u, 0);
        rsexp rhs = r_bytevector_new (r, 1u, 1);
        EXPECT_FALSE (equal_p (lhs, rhs));
    }
}

TEST_F (test_bytevector, r_bytevector_p)
{
    EXPECT_FALSE (r_bytevector_p (R_FALSE));
    EXPECT_TRUE (r_bytevector_p (r_bytevector_new (r, 0u, 0)));
}

TEST_F (test_bytevector, r_list_to_bytevector)
{
    // Empty list
    {
        rsexp expected = r_bytevector_new (r, 0u, 0);
        rsexp actual = r_list_to_bytevector (r, R_NULL);

        EXPECT_FALSE (r_failure_p (actual));
        EXPECT_TRUE (equal_p (expected, actual));
    }

    // Non-empty list
    {
        rsexp expected = r_bytevector_new (r, 2u, 0);
        r_bytevector_u8_set_x (r, expected, 1u, 1);

        rsexp actual = r_list_to_bytevector (r, r_list (r, 2, R_ZERO, R_ONE));

        EXPECT_FALSE (r_failure_p (actual));
        EXPECT_EQ (2, r_int_from_sexp (r_bytevector_length (actual)));
        EXPECT_TRUE (equal_p (expected, actual));
    }

    // List with non-byte element
    {
        rsexp list = r_list (r, 1, r_int_to_sexp (256));
        rsexp actual = r_list_to_bytevector (r, list);
        EXPECT_TRUE (r_failure_p (actual));
    }

    // Out-of-memory
    {
        rsexp list = r_list (r, 1, R_ZERO);

        allocator_disruptor ad;
        rsexp actual = r_list_to_bytevector (r, list);

        EXPECT_TRUE (r_failure_p (actual));
    }

    // Malformed list
    {
        rsexp actual = r_list_to_bytevector (r, R_TRUE);
        EXPECT_TRUE (r_failure_p (actual));
    }
}

TEST_F (test_bytevector, r_bytevector_u8_ref)
{
    {
        rsexp obj = r_bytevector_new (r, 1u, 0);
        EXPECT_EQ (R_ZERO, r_bytevector_u8_ref (r, obj, 0u));
    }

    {
        rsexp obj = r_bytevector_new (r, 0u, 0);
        EXPECT_TRUE (r_failure_p (r_bytevector_u8_ref (r, obj, 0u)));
    }
}

TEST_F (test_bytevector, r_bytevector_u8_set_x)
{
    {
        rsexp obj = r_bytevector_new (r, 1u, 0);
        EXPECT_FALSE (r_failure_p (r_bytevector_u8_set_x (r, obj, 0u, 0)));
    }

    {
        rsexp obj = r_bytevector_new (r, 0u, 0);
        EXPECT_TRUE (r_failure_p (r_bytevector_u8_set_x (r, obj, 0u, 0)));
    }
}
