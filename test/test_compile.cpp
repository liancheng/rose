#include "utils.hpp"

class test_compile : public fixture_base {
protected:
    rconstcstring from (rconstcstring program)
    {
        return to_cstr (compile (read_seq (program)));
    }
};

TEST_F (test_compile, halt)
{
    EXPECT_STREQ ("(halt)", from (""));
}

TEST_F (test_compile, reference)
{
    EXPECT_STREQ ("(refer x (halt))", from ("x "));
}

TEST_F (test_compile, constant)
{
    EXPECT_STREQ ("(const 1 (halt))", from ("1 "));
}

TEST_F (test_compile, quotation)
{
    EXPECT_STREQ ("(const x (halt))", from ("'x "));
    EXPECT_STREQ ("(const x (halt))", from ("(quote x)"));
}

TEST_F (test_compile, sequence)
{
    EXPECT_STREQ ("(refer x (refer y (halt)))", from ("x y "));
}

TEST_F (test_compile, assignment)
{
    EXPECT_STREQ ("(const 1 (assign x (halt)))", from ("(set! x 1)"));
}

TEST_F (test_compile, variable_definition)
{
    EXPECT_STREQ ("(const 1 (bind x (halt)))", from ("(define x 1)"));
}

TEST_F (test_compile, procedure_definition)
{
    EXPECT_STREQ ("(close () (const 1 (return)) (bind f (halt)))",
                  from ("(define (f) 1)"));

    EXPECT_STREQ ("(close x (const 1 (return)) (bind f (halt)))",
                  from ("(define (f . x) 1)"));

    EXPECT_STREQ ("(close (x . y) (const 1 (return)) (bind f (halt)))",
                  from ("(define (f x . y) 1)"));

    EXPECT_STREQ ("(close (x y) (const 1 (return)) (bind f (halt)))",
                  from ("(define (f x y) 1)"));
}

TEST_F (test_compile, conditional)
{
    /*
     * R_UNSPECIFIED cannot be read by r_read, so we must compose the
     * instruction DAG manually.
     */

    rsexp halt = emit_halt (r);
    rsexp expected =
        emit_const (r, R_FALSE,
                emit_branch (r,
                    emit_const (r, R_TRUE, halt),
                    emit_const (r, R_UNSPECIFIED, halt)));

    EXPECT_STREQ (to_cstr (expected), from ("(if #f #t)"));
}

TEST_F (test_compile, lambda)
{
    EXPECT_STREQ ("(close (x y) (refer x (return)) (halt))",
                  from ("(lambda (x y) x)"));

    EXPECT_STREQ ("(close x (refer x (return)) (halt))",
                  from ("(lambda x x)"));

    EXPECT_STREQ ("(close (x . y) (refer x (return)) (halt))",
                  from ("(lambda (x . y) x)"));
}
