#include "utils.hpp"

#include "rose/eq.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/symbol.h"

class test_reader : public fixture_base {
protected:
    virtual void SetUp ()
    {
        fixture_base::SetUp ();
    }

    virtual void TearDown ()
    {
        fixture_base::TearDown ();
    }

    void set_input (rconstcstring input)
    {
        rsexp port = r_open_input_string (state, r_string_new (state, input));
        r_set_current_input_port_x (state, port);
    }
};

TEST_F (test_reader, boolean)
{
    set_input ("#t ");
    EXPECT_TRUE (r_eq_p (state, R_TRUE, r_read (state)));

    set_input ("#f ");
    EXPECT_TRUE (r_eq_p (state, R_FALSE, r_read (state)));
}
