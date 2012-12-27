#include "utils.hpp"

class test_reader : public fixture_base {};

TEST_F (test_reader, boolean)
{
    EXPECT_STREQ ("#t", rdw ("#t "));
    EXPECT_STREQ ("#f", rdw ("#f "));
}

TEST_F (test_reader, integer)
{
    EXPECT_STREQ ("0", rdw ("0 "));
    EXPECT_STREQ ("1", rdw ("1 "));
    EXPECT_STREQ ("1", rdw ("+1 "));
    EXPECT_STREQ ("-1", rdw ("-1 "));
    EXPECT_STREQ ("2", rdw ("#b10 "));
    EXPECT_STREQ ("8", rdw ("#o10 "));
    EXPECT_STREQ ("10", rdw ("#d10 "));
    EXPECT_STREQ ("16", rdw ("#x10 "));
    EXPECT_STREQ ("2", rdw ("#e#b10 "));
    EXPECT_STREQ ("10", rdw ("#d#e10 "));
}

TEST_F (test_reader, decimal)
{
    EXPECT_STREQ ("0.100000", rdw (".1 "));
    EXPECT_STREQ ("1.000000", rdw ("1.0 "));
}

TEST_F (test_reader, rational)
{
    EXPECT_STREQ ("0", rdw ("0/1 "));
    EXPECT_STREQ ("1/2", rdw ("1/2 "));
    EXPECT_STREQ ("1/2", rdw ("2/4 "));
    EXPECT_STREQ ("-1/2", rdw ("-2/4 "));
    EXPECT_STREQ ("6/5", rdw ("#e1.2 "));
}

TEST_F (test_reader, pure_imaginary)
{
    EXPECT_STREQ ("0+1i", rdw ("+i "));
    EXPECT_STREQ ("0-1i", rdw ("-i "));
    EXPECT_STREQ ("0+2i", rdw ("+2i "));
    EXPECT_STREQ ("0-2i", rdw ("-2i "));
}

TEST_F (test_reader, complex_number)
{
    EXPECT_STREQ ("1+1i", rdw ("1+i "));
    EXPECT_STREQ ("0-1i", rdw ("0-1i "));
    EXPECT_STREQ ("16-16i", rdw ("#x10-10i "));
}

TEST_F (test_reader, string)
{
    EXPECT_STREQ ("\"\"", rdw ("\"\""));
    EXPECT_STREQ ("\"\\\"\"", rdw ("\"\\\"\""));
}

TEST_F (test_reader, abbreviated_list)
{
    EXPECT_STREQ ("(quote a)", rdw ("'a "));
    EXPECT_STREQ ("(quasiquote a)", rdw ("`a "));
    EXPECT_STREQ ("(unquote a)", rdw (",a "));
    EXPECT_STREQ ("(unquote-splicing a)", rdw (",@a "));
}

TEST_F (test_reader, full_list)
{
    EXPECT_STREQ ("()", rdw ("()"));
    EXPECT_STREQ ("(a)", rdw ("(a)"));
    EXPECT_STREQ ("(a . b)", rdw ("(a . b)"));
    EXPECT_STREQ ("(a b . c)", rdw ("(a b . c)"));
    EXPECT_STREQ ("(a b)", rdw ("(a b)"));
    EXPECT_STREQ ("(a b)", rdw ("(a b . ())"));
}

TEST_F (test_reader, vector)
{
    EXPECT_STREQ ("#()", rdw ("#()"));
    EXPECT_STREQ ("#(a)", rdw ("#(a)"));
    EXPECT_STREQ ("#(a b)", rdw ("#(a b)"));
}

TEST_F (test_reader, character)
{
    EXPECT_STREQ ("#\\a", rdw ("#\\a "));
    EXPECT_STREQ ("#\\space", rdw ("#\\space "));
    EXPECT_STREQ ("#\\tab", rdw ("#\\tab "));
    EXPECT_STREQ ("#\\newline", rdw ("#\\newline "));
    EXPECT_STREQ ("#\\backspace", rdw ("#\\backspace "));
    EXPECT_STREQ ("#\\delete", rdw ("#\\delete "));
    EXPECT_STREQ ("#\\return", rdw ("#\\return "));
    EXPECT_STREQ ("#\\escape", rdw ("#\\escape "));
}
