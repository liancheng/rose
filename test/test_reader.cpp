#include "utils.hpp"

#include "rose/eq.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/symbol.h"

class test_reader : public fixture_base {
protected:
    rsexp set_input (rconstcstring input)
    {
        rsexp port = r_open_input_string (state, r_string_new (state, input));
        return r_reader_new (state, port);
    }

    rsexp reader;
};

TEST_F (test_reader, boolean)
{
    EXPECT_TRUE (r_eq_p (state, R_TRUE,  r_read (set_input ("#t "))));
    EXPECT_TRUE (r_eq_p (state, R_FALSE, r_read (set_input ("#f "))));
}
