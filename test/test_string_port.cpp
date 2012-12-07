#include "rose/eq.h"
#include "rose/port.h"
#include "rose/state.h"
#include "rose/string.h"

#include <gtest/gtest.h>

class test_output_string_port : public testing::Test {
protected:
    virtual void SetUp ()
    {
        state = r_state_open ();
        port = r_open_output_string (state);
    }

    virtual void TearDown ()
    {
        r_state_free (state);
    }

    RState* state;
    rsexp port;
};

TEST_F (test_output_string_port, empty)
{
    rsexp expected = r_string_new (state, "");
    rsexp actual   = r_get_output_string (state, port);

    EXPECT_TRUE (r_equal_p (state, expected, actual));
}

TEST_F (test_output_string_port, write_once)
{
    r_port_display (state, port, R_TRUE);

    rsexp expected = r_string_new (state, "#t");
    rsexp actual   = r_get_output_string (state, port);

    EXPECT_TRUE (r_equal_p (state, expected, actual));
}

TEST_F (test_output_string_port, write_twice)
{
    {
        r_port_display (state, port, r_string_new (state, "hello"));

        rsexp expected = r_string_new (state, "hello");
        rsexp actual   = r_get_output_string (state, port);

        EXPECT_TRUE (r_equal_p (state, expected, actual));
    }

    {
        r_port_display (state, port, r_string_new (state, " world"));

        rsexp expected = r_string_new (state, "hello world");
        rsexp actual   = r_get_output_string (state, port);

        EXPECT_TRUE (r_equal_p (state, expected, actual));
    }
}
