#include "utils.hpp"

class test_reader : public fixture_base {
protected:
    rconstcstring from (rconstcstring datum)
    {
        return to_cstr (read (datum));
    }
};

TEST_F (test_reader, comment)
{
    EXPECT_STREQ ("#<eof>", from ("#; 1 "));
}

TEST_F (test_reader, boolean)
{
    EXPECT_STREQ ("#t", from ("#t "));
    EXPECT_STREQ ("#f", from ("#f "));
}

TEST_F (test_reader, integer)
{
    EXPECT_STREQ ("0",  from ("0 "));
    EXPECT_STREQ ("1",  from ("1 "));
    EXPECT_STREQ ("1",  from ("+1 "));
    EXPECT_STREQ ("-1", from ("-1 "));
    EXPECT_STREQ ("2",  from ("#b10 "));
    EXPECT_STREQ ("8",  from ("#o10 "));
    EXPECT_STREQ ("10", from ("#d10 "));
    EXPECT_STREQ ("16", from ("#x10 "));
    EXPECT_STREQ ("2",  from ("#e#b10 "));
    EXPECT_STREQ ("10", from ("#d#e10 "));
}

TEST_F (test_reader, decimal)
{
    EXPECT_STREQ ("0.100000", from (".1 "));
    EXPECT_STREQ ("1.000000", from ("1.0 "));
}

TEST_F (test_reader, rational)
{
    EXPECT_STREQ ("0",    from ("0/1 "));
    EXPECT_STREQ ("1/2",  from ("1/2 "));
    EXPECT_STREQ ("1/2",  from ("2/4 "));
    EXPECT_STREQ ("-1/2", from ("-2/4 "));
    EXPECT_STREQ ("6/5",  from ("#e1.2 "));
}

TEST_F (test_reader, pure_imaginary)
{
    EXPECT_STREQ ("0+1i", from ("+i "));
    EXPECT_STREQ ("0-1i", from ("-i "));
    EXPECT_STREQ ("0+2i", from ("+2i "));
    EXPECT_STREQ ("0-2i", from ("-2i "));
}

TEST_F (test_reader, complex_number)
{
    EXPECT_STREQ ("1+1i",   from ("1+i "));
    EXPECT_STREQ ("0-1i",   from ("0-1i "));
    EXPECT_STREQ ("16-16i", from ("#x10-10i "));
}

TEST_F (test_reader, string)
{
    EXPECT_STREQ ("\"\"",     from ("\"\""));
    EXPECT_STREQ ("\"\\\"\"", from ("\"\\\"\""));
}

TEST_F (test_reader, abbreviated_list)
{
    EXPECT_STREQ ("(quote a)",            from ("'a "));
    EXPECT_STREQ ("(quasiquote a)",       from ("`a "));
    EXPECT_STREQ ("(unquote a)",          from (",a "));
    EXPECT_STREQ ("(unquote-splicing a)", from (",@a "));
}

TEST_F (test_reader, full_list)
{
    EXPECT_STREQ ("()",        from ("()"));
    EXPECT_STREQ ("(a)",       from ("(a)"));
    EXPECT_STREQ ("(a . b)",   from ("(a . b)"));
    EXPECT_STREQ ("(a b . c)", from ("(a b . c)"));
    EXPECT_STREQ ("(a b)",     from ("(a b)"));
    EXPECT_STREQ ("(a b)",     from ("(a b . ())"));
}

TEST_F (test_reader, vector)
{
    EXPECT_STREQ ("#()",    from ("#()"));
    EXPECT_STREQ ("#(a)",   from ("#(a)"));
    EXPECT_STREQ ("#(a b)", from ("#(a b)"));
}

TEST_F (test_reader, character)
{
    EXPECT_STREQ ("#\\a",         from ("#\\a "));
    EXPECT_STREQ ("#\\space",     from ("#\\space "));
    EXPECT_STREQ ("#\\tab",       from ("#\\tab "));
    EXPECT_STREQ ("#\\newline",   from ("#\\newline "));
    EXPECT_STREQ ("#\\backspace", from ("#\\backspace "));
    EXPECT_STREQ ("#\\delete",    from ("#\\delete "));
    EXPECT_STREQ ("#\\return",    from ("#\\return "));
    EXPECT_STREQ ("#\\escape",    from ("#\\escape "));
    EXPECT_STREQ ("#\\null",      from ("#\\null "));
    EXPECT_STREQ ("#\\alarm",     from ("#\\alarm "));
}

TEST_F (test_reader, bytevector)
{
    EXPECT_STREQ ("#u8()",      from ("#u8()"));
    EXPECT_STREQ ("#u8(0)",     from ("#u8(0)"));
    EXPECT_STREQ ("#u8(0 255)", from ("#u8(0 255)"));
    EXPECT_STREQ ("#<failure>", from ("#u8(256)"));
}
