#include <cstring>
#include <gc/gc.h>
#include <gmpxx.h>
#include <gtest/gtest.h>

extern "C" {

#include "detail/number.h"
#include "detail/number_reader.h"

}

using std::string;
using testing::Test;
using testing::AssertionResult;
using testing::AssertionSuccess;
using testing::AssertionFailure;

class number_reader_test : public Test {
protected:
    virtual void SetUp ()
    {
        reader = r_number_reader_new ();
    }

    virtual void TearDown ()
    {
        GC_FREE (reader);
    }

    void feed (char const* text)
    {
        r_number_reader_feed_input (reader, text);
    }

    AssertionResult exactly_equal (RFixnum const& expected,
                                   string const&  lexeme)
    {
        rsexp    number = r_number_read (this->reader, lexeme.c_str());
        RFixnum& actual = *FIXNUM_FROM_SEXP (number);

        return mpq_class (expected.real) == mpq_class (actual.real) && 
               mpq_class (expected.imag) == mpq_class (actual.imag)
               ? AssertionSuccess ()
               : AssertionFailure ();
    }

    RNumberReader* reader;
};

TEST_F (number_reader_test, initial_state)
{
    ASSERT_TRUE (reader->exact);
    ASSERT_EQ (10, reader->radix);
}

TEST_F (number_reader_test, consume)
{
    feed ("bar");
    r_number_reader_consume (reader, 1u);
    ASSERT_EQ (reader->begin + 1, reader->pos);
}

TEST_F (number_reader_test, consume_overflow)
{
    feed ("bar");
    r_number_reader_consume (reader, 4u);
    ASSERT_EQ (reader->end, reader->pos);
}

TEST_F (number_reader_test, lookahead)
{
    feed ("bar");
    ASSERT_EQ ('b', r_number_reader_lookahead (reader, 0u));
}

TEST_F (number_reader_test, lookahead_overflow)
{
    feed ("bar");
    ASSERT_EQ ('\0', r_number_reader_lookahead (reader, 4u));
}

TEST_F (number_reader_test, next)
{
    feed ("bar");
    ASSERT_EQ ('b', r_number_reader_next (reader));
    ASSERT_EQ ('a', r_number_reader_next (reader));
}

TEST_F (number_reader_test, next_overflow)
{
    feed ("bar");

    for (rint i = 0; i < 3; ++i)
        r_number_reader_next (reader);

    ASSERT_EQ ('\0', r_number_reader_next (reader));
}

TEST_F (number_reader_test, eoi_p)
{
    feed ("xx");

    ASSERT_FALSE (r_number_reader_eoi_p (reader));

    r_number_reader_next (reader);
    ASSERT_FALSE (r_number_reader_eoi_p (reader));

    r_number_reader_next (reader);
    ASSERT_TRUE (r_number_reader_eoi_p (reader));

    r_number_reader_next (reader);
    ASSERT_TRUE (r_number_reader_eoi_p (reader));
}

TEST_F (number_reader_test, mark_rewind_begin)
{
    char const* mark;

    feed ("xx");
    mark = r_number_reader_mark (reader);
    ASSERT_EQ (reader->begin, mark);

    r_number_reader_next (reader);
    ASSERT_EQ (reader->begin + 1, reader->pos);

    r_number_reader_rewind (reader, mark);
    ASSERT_EQ (mark, reader->pos);
}

TEST_F (number_reader_test, read_prefix_with_exactness_only)
{
    feed ("#i");
    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_FALSE (reader->exact);
    ASSERT_EQ (10, reader->radix);
}

TEST_F (number_reader_test, read_prefix_with_exactness_and_radix)
{
    feed ("#i#b");
    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_FALSE (reader->exact);
    ASSERT_EQ (2, reader->radix);
}

TEST_F (number_reader_test, read_prefix_with_radix_only)
{
    feed ("#b");
    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_TRUE (reader->exact);
    ASSERT_EQ (2, reader->radix);
}

TEST_F (number_reader_test, read_empty_prefix)
{
    feed ("");
    ASSERT_TRUE (r_number_read_prefix (reader));
    ASSERT_TRUE (reader->exact);
    ASSERT_EQ (10, reader->radix);
}

TEST_F (number_reader_test, read_digit)
{
    ruint digit;

    feed ("#b012");
    ASSERT_TRUE (r_number_read_radix (reader));
    ASSERT_TRUE (r_number_read_digit (reader, &digit));
    ASSERT_EQ (0u, digit);
    ASSERT_TRUE (r_number_read_digit (reader, &digit));
    ASSERT_EQ (1u, digit);
    ASSERT_FALSE (r_number_read_digit (reader, &digit));
}
