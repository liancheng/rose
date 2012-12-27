#include "utils.hpp"

class test_compile : public fixture_base {};

TEST_F (test_compile, halt)
{
    EXPECT_STREQ ("(halt)", rcw (""));
}

TEST_F (test_compile, refer)
{
    EXPECT_STREQ ("(refer x (halt))", rcw ("x "));
}

TEST_F (test_compile, constant)
{
    EXPECT_STREQ ("(const 1 (halt))", rcw ("1 "));
}

TEST_F (test_compile, quote)
{
    EXPECT_STREQ ("(const x (halt))", rcw ("'x "));
}

TEST_F (test_compile, sequence)
{
    EXPECT_STREQ ("(refer x (refer y (halt)))", rcw ("x y "));
}

TEST_F (test_compile, assign)
{
    EXPECT_STREQ ("(const 1 (assign x (halt)))", rcw ("(set! x 1)"));
}

TEST_F (test_compile, define)
{
    EXPECT_STREQ ("(const 1 (bind x (halt)))", rcw ("(define x 1)"));
}

TEST_F (test_compile, conditional)
{
    rsexp halt = emit_halt (state);
    rsexp expected =
        emit_const (state, R_FALSE,
                emit_branch (state,
                    emit_const (state, R_TRUE, halt),
                    emit_const (state, R_UNSPECIFIED, halt)));

    EXPECT_STREQ (w (expected), rcw ("(if #f #t)"));
}

TEST_F (test_compile, lambda)
{
    EXPECT_STREQ ("(close (x) (refer x (return)) (halt))",
                  rcw ("(lambda (x) x)"));
}
