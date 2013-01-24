#include "utils.hpp"

#include "rose/eq.h"
#include "rose/pair.h"
#include "rose/vector.h"

class test_pair : public fixture_base {};

TEST_F (test_pair, r_reverse_null)
{
    rsexp expected = R_NULL;
    rsexp actual   = r_reverse_x (r, R_NULL);

    EXPECT_EQ (expected, actual);
}

TEST_F (test_pair, r_reverse_single_element)
{
    rsexp expected = r_list (r, 1, R_TRUE);
    rsexp actual   = r_reverse_x (r, r_list (r, 1, R_TRUE));

    EXPECT_TRUE (equal_p (expected, actual));
}

TEST_F (test_pair, r_reverse_multiple_elements)
{
    rsexp expected = r_list (r, 2, R_FALSE, R_TRUE);
    rsexp actual   = r_reverse_x (r, r_list (r, 2, R_TRUE, R_FALSE));

    EXPECT_TRUE (equal_p (expected, actual));
}
