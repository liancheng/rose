#include "utils.hpp"

#include "rose/number.h"

class test_number_reader : public fixture_base {};

TEST_F (test_number_reader, string_to_number)
{
    EXPECT_TRUE (r_exact_p (r_string_to_number (r, "#e1e128")));
    EXPECT_TRUE (r_exact_p (r_string_to_number (r, "1/2")));
    EXPECT_TRUE (r_exact_p (r_string_to_number (r, "#e1.2")));
}
