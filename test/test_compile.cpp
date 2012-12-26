#include "utils.hpp"

#include "detail/compile.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/string.h"

class test_compile : public fixture_base {
protected:
    rconstcstring cw (rconstcstring source)
    {
        rsexp str     = r_string_new (state, source);
        rsexp port    = r_open_input_string (state, str);
        rsexp reader  = r_reader_new (state, port);
        rsexp program = R_NULL;

        while (true) {
            rsexp datum = r_read (reader);

            if (r_eof_object_p (datum))
                break;

            program = r_cons (state, datum, program);
        }

        return write (r_compile (state, program));
    }

    rconstcstring rw (rconstcstring input)
    {
        return write (r_read_from_string (state, input));
    }

    rconstcstring write (rsexp obj)
    {
        rsexp out = r_open_output_string (state);
        r_port_write (state, out, obj);
        return r_string_to_cstr (r_get_output_string (state, out));
    }
};

TEST_F (test_compile, halt)
{
    EXPECT_STREQ (rw ("(halt)"), cw (""));
}

TEST_F (test_compile, refer)
{
    EXPECT_STREQ (rw ("(refer x (halt))"), cw ("x "));
}

TEST_F (test_compile, constant)
{
    EXPECT_STREQ (rw ("(const 1 (halt))"), cw ("1 "));
}

TEST_F (test_compile, quote)
{
    EXPECT_STREQ (rw ("(const x (halt))"), cw ("'x "));
}

TEST_F (test_compile, sequence)
{
    EXPECT_STREQ (rw ("(refer x (refer y (halt)))"), cw ("x y "));
}

TEST_F (test_compile, assign)
{
    EXPECT_STREQ (rw ("(const 1 (assign x (halt)))"), cw ("(set! x 1)"));
}

TEST_F (test_compile, define)
{
    EXPECT_STREQ (rw ("(const 1 (bind x (halt)))"), cw ("(define x 1)"));
}
