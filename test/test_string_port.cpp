#include "utils.hpp"

#include "rose/eq.h"
#include "rose/port.h"
#include "rose/string.h"

class test_output_string_port : public fixture_base {
protected:
    virtual void SetUp ()
    {
        fixture_base::SetUp ();
        port = r_open_output_string (state);
    }

    virtual void TearDown ()
    {
        r_close_port (port);
        fixture_base::TearDown ();
    }

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
    rsexp actual = r_get_output_string (state, port);

    EXPECT_TRUE (r_equal_p (state, expected, actual));
}

TEST_F (test_output_string_port, write_twice)
{
    {
        r_port_display (state, port, r_string_new (state, "hello"));

        rsexp expected = r_string_new (state, "hello");
        rsexp actual = r_get_output_string (state, port);

        EXPECT_TRUE (r_equal_p (state, expected, actual));
    }

    {
        r_port_display (state, port, r_string_new (state, " world"));

        rsexp expected = r_string_new (state, "hello world");
        rsexp actual = r_get_output_string (state, port);

        EXPECT_TRUE (r_equal_p (state, expected, actual));
    }
}

class test_input_string_port : public fixture_base {
protected:
    rsexp new_port (rconstcstring input)
    {
        return r_open_input_string (state, r_string_new (state, input));
    }
};

TEST_F (test_input_string_port, empty)
{
    rsexp port = new_port ("");
    EXPECT_TRUE (r_eof_object_p (r_port_read_char (state, port)));
    EXPECT_TRUE (r_eof_p (port));
}

TEST_F (test_input_string_port, read_once)
{
    rsexp port = new_port ("a");
    EXPECT_EQ (r_char_to_sexp ('a'), r_port_read_char (state, port));
}

TEST_F (test_input_string_port, read_twice)
{
    rsexp port = new_port ("ab");
    EXPECT_EQ (r_char_to_sexp ('a'), r_port_read_char (state, port));
    EXPECT_EQ (r_char_to_sexp ('b'), r_port_read_char (state, port));
}

TEST_F (test_input_string_port, read_to_eof)
{
    rsexp port = new_port ("a");
    EXPECT_EQ (r_char_to_sexp ('a'), r_port_read_char (state, port));
    EXPECT_TRUE (r_eof_object_p (r_port_read_char (state, port)));
    EXPECT_TRUE (r_eof_p (port));
}
