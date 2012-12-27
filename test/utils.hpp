#ifndef __ROSE_TEST_UTILS_HPP__
#define __ROSE_TEST_UTILS_HPP__

#include "detail/compile.h"
#include "rose/port.h"
#include "rose/reader.h"
#include "rose/state.h"
#include "rose/string.h"

#include <gtest/gtest.h>

class fixture_base : public testing::Test {
protected:
    RState* state;

    virtual void SetUp ()
    {
        state = r_state_open ();
    }

    virtual void TearDown ()
    {
        r_state_free (state);
    }

    rsexp r (rsexp port)
    {
        rsexp reader  = r_reader_new (state, port);
        rsexp program = R_NULL;

        while (true) {
            rsexp datum = r_read (reader);

            if (r_eof_object_p (datum))
                break;

            program = r_cons (state, datum, program);
        }

        return program;
    }

    rsexp r (rconstcstring source)
    {
        rsexp str  = r_string_new (state, source);
        rsexp port = r_open_input_string (state, str);

        return r (port);
    }

    rsexp rd (rsexp port)
    {
        return r_read (r_reader_new (state, port));
    }

    rsexp rd (rconstcstring input)
    {
        rsexp port = r_open_input_string (state, r_string_new (state, input));
        return rd (port);
    }

    rsexp c (rsexp program)
    {
        return r_compile (state, program);
    }

    rconstcstring w (rsexp obj)
    {
        rsexp out = r_open_output_string (state);
        r_port_write (state, out, obj);
        return r_string_to_cstr (r_get_output_string (state, out));
    }

    rconstcstring rdw (rconstcstring source)
    {
        return w (rd (source));
    }

    rsexp rc (rsexp port)
    {
        return c (r (port));
    }

    rconstcstring rcw (rconstcstring source)
    {
        return w (c (r (source)));
    }
};

#endif  //  __ROSE_TEST_UTILS_HPP__
