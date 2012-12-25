#include "utils.hpp"

#include "rose/eq.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"
#include "rose/symbol.h"

class test_reader : public fixture_base {
protected:
    rconstcstring rw (rconstcstring input)
    {
        rsexp out = r_open_output_string (state);
        r_port_write (state, out, r_read_from_string (state, input));
        return r_string_to_cstr (r_get_output_string (state, out));
    }
};

TEST_F (test_reader, boolean)
{
    EXPECT_STREQ ("#t", rw ("#t "));
    EXPECT_STREQ ("#f", rw ("#f "));
}

TEST_F (test_reader, integer)
{
    EXPECT_STREQ ("0", rw ("0 "));
    EXPECT_STREQ ("1", rw ("1 "));
    EXPECT_STREQ ("1", rw ("+1 "));
    EXPECT_STREQ ("-1", rw ("-1 "));
    EXPECT_STREQ ("2", rw ("#b10 "));
    EXPECT_STREQ ("8", rw ("#o10 "));
    EXPECT_STREQ ("10", rw ("#d10 "));
    EXPECT_STREQ ("16", rw ("#x10 "));
    EXPECT_STREQ ("2", rw ("#e#b10 "));
    EXPECT_STREQ ("10", rw ("#d#e10 "));
}

TEST_F (test_reader, decimal)
{
    EXPECT_STREQ ("0.100000", rw (".1 "));
    EXPECT_STREQ ("1.000000", rw ("1.0 "));
}

TEST_F (test_reader, rational)
{
    EXPECT_STREQ ("0", rw ("0/1 "));
    EXPECT_STREQ ("1/2", rw ("1/2 "));
    EXPECT_STREQ ("1/2", rw ("2/4 "));
    EXPECT_STREQ ("-1/2", rw ("-2/4 "));
    EXPECT_STREQ ("6/5", rw ("#e1.2 "));
}

TEST_F (test_reader, pure_imaginary)
{
    EXPECT_STREQ ("0+1i", rw ("+i "));
    EXPECT_STREQ ("0-1i", rw ("-i "));
    EXPECT_STREQ ("0+2i", rw ("+2i "));
    EXPECT_STREQ ("0-2i", rw ("-2i "));
}

TEST_F (test_reader, complex_number)
{
    EXPECT_STREQ ("1+1i", rw ("1+i "));
    EXPECT_STREQ ("0-1i", rw ("0-1i "));
    EXPECT_STREQ ("16-16i", rw ("#x10-10i "));
}

TEST_F (test_reader, string)
{
    EXPECT_STREQ ("\"\"", rw ("\"\" "));
    EXPECT_STREQ ("\"hello \\\"world\\\"\"", rw ("\"hello \\\"world\\\"\""));
}

TEST_F (test_reader, abbreviated_list)
{
    EXPECT_STREQ ("(quote a)", rw ("'a "));
    EXPECT_STREQ ("(quasiquote a)", rw ("`a "));
    EXPECT_STREQ ("(unquote a)", rw (",a "));
    EXPECT_STREQ ("(unquote-splicing a)", rw (",@a "));
}

TEST_F (test_reader, full_list)
{
    EXPECT_STREQ ("()", rw ("() "));
    EXPECT_STREQ ("(a)", rw ("(a) "));
    EXPECT_STREQ ("(a . b)", rw ("(a . b) "));
    EXPECT_STREQ ("(a b . c)", rw ("(a b . c) "));
    EXPECT_STREQ ("(a b)", rw ("(a b) "));
    EXPECT_STREQ ("(a b)", rw ("(a b . ()) "));
}

TEST_F (test_reader, vector)
{
    EXPECT_STREQ ("#()", rw ("#() "));
    EXPECT_STREQ ("#(a)", rw ("#(a) "));
    EXPECT_STREQ ("#(a b)", rw ("#(a b) "));
}

TEST_F (test_reader, character)
{
    EXPECT_STREQ ("#\\a", rw ("#\\a "));
    EXPECT_STREQ ("#\\space", rw ("#\\space "));
    EXPECT_STREQ ("#\\tab", rw ("#\\tab "));
    EXPECT_STREQ ("#\\newline", rw ("#\\newline "));
    EXPECT_STREQ ("#\\backspace", rw ("#\\backspace "));
    EXPECT_STREQ ("#\\delete", rw ("#\\delete "));
    EXPECT_STREQ ("#\\return", rw ("#\\return "));
    EXPECT_STREQ ("#\\escape", rw ("#\\escape "));
}
