#include <cstring>
#include <gmpxx.h>
#include <gtest/gtest.h>

extern "C" {

#include "detail/number.h"

}

class number_reader_test : public testing::Test {
protected:
    RNumberReader* reader;

    virtual void SetUp ()
    {
        reader = r_number_reader_new ();
    }

    virtual void TearDown ()
    {
    }

    virtual void set_input (char const* input)
    {
        char const* begin = input;
        char const* end   = input + strlen (input);

        r_number_reader_set_input (reader, begin, end);
    }
};

TEST_F (number_reader_test, initial_state)
{
    ASSERT_EQ (10, reader->radix);
    ASSERT_TRUE (reader->exact);
}

TEST_F (number_reader_test, read_exactness_exact)
{
    set_input ("#e");

    ASSERT_TRUE (r_number_read_exactness (reader));
    ASSERT_TRUE (reader->exact);
}

TEST_F (number_reader_test, read_exactness_inexact)
{
    set_input ("#i");

    ASSERT_TRUE (r_number_read_exactness (reader));
    ASSERT_FALSE (reader->exact);
}

TEST_F (number_reader_test, read_exactness_error)
{
    set_input ("foo");

    ASSERT_FALSE (r_number_read_exactness (reader));
    ASSERT_EQ ('f', r_number_lookahead (reader, 0));
    ASSERT_EQ ('o', r_number_lookahead (reader, 1));
    ASSERT_EQ ('o', *(reader->pos));
}

TEST_F (number_reader_test, read_exactness_all_consumed)
{
    set_input ("#i");

    ASSERT_TRUE (r_number_read_exactness (reader));
    ASSERT_TRUE (r_number_eoi_p (reader));
}

TEST_F (number_reader_test, read_radix_bin)
{
    set_input ("#b");

    ASSERT_TRUE (r_number_read_radix (reader));
    ASSERT_EQ (2, reader->radix);
}

TEST_F (number_reader_test, read_radix_oct)
{
    set_input ("#o");

    ASSERT_TRUE (r_number_read_radix (reader));
    ASSERT_EQ (8, reader->radix);
}

TEST_F (number_reader_test, read_radix_dec)
{
    set_input ("#d");

    ASSERT_TRUE (r_number_read_radix (reader));
    ASSERT_EQ (10, reader->radix);
}

TEST_F (number_reader_test, read_radix_hex)
{
    set_input ("#x");

    ASSERT_TRUE (r_number_read_radix (reader));
    ASSERT_EQ (16, reader->radix);
    ASSERT_EQ (reader->end, reader->pos);
}

TEST_F (number_reader_test, read_radix_error)
{
    set_input ("foo");

    ASSERT_FALSE (r_number_read_radix (reader));
    ASSERT_EQ ('f', r_number_lookahead (reader, 0));
    ASSERT_EQ ('o', r_number_lookahead (reader, 1));
    ASSERT_EQ ('o', *(reader->pos));
}

TEST_F (number_reader_test, read_prefix)
{
    set_input ("#i#b");

    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_FALSE (reader->exact);
    ASSERT_EQ (2, reader->radix);
    ASSERT_TRUE (r_number_eoi_p (reader));
}

TEST_F (number_reader_test, read_prefix_omit_radix)
{
    set_input ("#i");

    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_FALSE (reader->exact);
    ASSERT_EQ (10, reader->radix);
}

TEST_F (number_reader_test, read_prefix_omit_exactness)
{
    set_input ("#b");

    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_TRUE (reader->exact);
    ASSERT_EQ (2, reader->radix);
}

TEST_F (number_reader_test, read_prefix_omit_radix_and_exactness)
{
    set_input ("");

    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_TRUE (reader->exact);
    ASSERT_EQ (10, reader->radix);
}

TEST_F (number_reader_test, read_prefix_error)
{
    set_input ("foo");

    // <prefix> is optional, so the ASSERT_TRUE assertion below should pass.
    ASSERT_TRUE (r_number_read_prefix (reader));

    ASSERT_EQ ('f', r_number_lookahead (reader, 0));
    ASSERT_EQ ('o', r_number_lookahead (reader, 1));
    ASSERT_EQ ('o', *(reader->pos));
}

TEST_F (number_reader_test, read_prefix_all_consumed)
{
    set_input ("#i#b");

    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_TRUE (r_number_eoi_p (reader));
}
