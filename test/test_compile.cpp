#include "detail/symbol.h"

#include "utils.hpp"

class test_compile : public fixture_base {
protected:
    rconstcstring expected (rconstcstring str)
    {
        return str;
    }

    rconstcstring actual (rconstcstring program)
    {
        return to_cstr (compile (read_seq (program)));
    }
};

TEST_F (test_compile, invalid_input)
{
    EXPECT_STREQ (expected ("#<failure>"),
                  to_cstr (r_compile_from_cstr (r, "(invalid-input")));
}

TEST_F (test_compile, halt)
{
    EXPECT_STREQ (expected ("(halt)"), actual (""));
}

TEST_F (test_compile, reference)
{
    EXPECT_STREQ (expected ("(refer x (halt))"), actual ("x "));
}

TEST_F (test_compile, constant)
{
    EXPECT_STREQ (expected ("(constant 1 (halt))"), actual ("1 "));
}

TEST_F (test_compile, quotation)
{
    EXPECT_STREQ (expected ("(constant x (halt))"), actual ("'x "));
    EXPECT_STREQ (expected ("(constant x (halt))"), actual ("(quote x)"));
    EXPECT_STREQ (expected ("#<failure>"), actual ("(quote)"));
}

TEST_F (test_compile, sequence)
{
    EXPECT_STREQ
        (expected ("(refer x (refer y (halt)))"),
         actual ("x y "));
}

TEST_F (test_compile, assignment)
{
    EXPECT_STREQ
        (expected ("(constant 1 (assign x (halt)))"),
         actual ("(set! x 1)"));

    EXPECT_STREQ (expected ("#<failure>"), actual ("(set!)"));
    EXPECT_STREQ (expected ("#<failure>"), actual ("(set! #t 1)"));
}

TEST_F (test_compile, variable_definition)
{
    EXPECT_STREQ
        (expected ("(constant 1 (bind x (halt)))"),
         actual ("(define x 1)"));

    EXPECT_STREQ (expected ("#<failure>"), actual ("(define)"));
    EXPECT_STREQ (expected ("#<failure>"), actual ("(define x 1 2)"));
}

TEST_F (test_compile, procedure_definition)
{
    EXPECT_STREQ
        (expected ("(close () (constant 1 (return)) (bind f (halt)))"),
         actual ("(define (f) 1)"));

    EXPECT_STREQ
        (expected ("(close x (constant 1 (return)) (bind f (halt)))"),
         actual ("(define (f . x) 1)"));

    EXPECT_STREQ
        (expected ("(close (x . y) (constant 1 (return)) (bind f (halt)))"),
         actual ("(define (f x . y) 1)"));

    EXPECT_STREQ
        (expected ("(close (x y) (constant 1 (return)) (bind f (halt)))"),
         actual ("(define (f x y) 1)"));
}

TEST_F (test_compile, conditional)
{
    {
        /*
         * R_UNSPECIFIED cannot be read by r_read, so we must compose the
         * instruction DAG manually.
         */

        rsexp halt = emit_halt (r);
        rsexp expected =
            emit_constant (r, R_FALSE,
                    emit_branch (r,
                        emit_constant (r, R_TRUE, halt),
                        emit_constant (r, R_UNSPECIFIED, halt)));

        EXPECT_STREQ (to_cstr (expected), actual ("(if #f #t)"));
    }

    EXPECT_STREQ (expected ("#<failure>"), actual ("(if . #t)"));
    EXPECT_STREQ (expected ("#<failure>"), actual ("(if #t)"));
}

TEST_F (test_compile, lambda)
{
    EXPECT_STREQ
        (expected ("(close (x y) (refer x (return)) (halt))"),
         actual ("(lambda (x y) x)"));

    EXPECT_STREQ
        (expected ("(close x (refer x (return)) (halt))"),
         actual ("(lambda x x)"));

    EXPECT_STREQ
        (expected ("(close (x . y) (refer x (return)) (halt))"),
         actual ("(lambda (x . y) x)"));
}

TEST_F (test_compile, call_cc)
{
    EXPECT_STREQ
        (expected ("(frame (halt) (capture-cc (arg "
                   "(close (k) (refer k (return)) (apply)))))"),
         actual ("(call/cc (lambda (k) k))"));

    EXPECT_STREQ (expected ("#<failure>"), actual ("(call/cc)"));
}

TEST_F (test_compile, application)
{
    EXPECT_STREQ
        (expected ("(frame (halt) (refer foo (apply)))"),
         actual ("(foo)"));

    EXPECT_STREQ
        (expected ("(frame (halt) (constant 1 (arg (refer foo (apply)))))"),
         actual ("(foo 1)"));

    EXPECT_STREQ (expected ("#<failure>"), actual ("(foo . 1)"));
}
