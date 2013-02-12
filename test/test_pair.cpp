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

TEST_F (test_pair, r_properfy)
{
    {
        rsexp input = read ("(1 2 3)");
        rsexp actual = r_properfy (r, input);

        EXPECT_FALSE (r_failure_p (actual));
        EXPECT_STREQ ("(1 2 3)", to_cstr (r_car (actual)));
        EXPECT_STREQ ("()", to_cstr (r_cdr (actual)));
    }

    {
        rsexp input = read ("(1 2 . 3)");
        rsexp actual = r_properfy (r, input);

        EXPECT_FALSE (r_failure_p (actual));
        EXPECT_STREQ ("(1 2)", to_cstr (r_car (actual)));
        EXPECT_STREQ ("3", to_cstr (r_cdr (actual)));
    }
}
