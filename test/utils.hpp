#ifndef __ROSE_TEST_UTILS_HPP__
#define __ROSE_TEST_UTILS_HPP__

#include "detail/compile.h"
#include "rose/eq.h"
#include "rose/io.h"
#include "rose/read.h"
#include "rose/state.h"
#include "rose/string.h"

#include <gtest/gtest.h>

class fixture_base : public testing::Test {
protected:
    RState* r;

    virtual void SetUp ()
    {
        r = r_state_new (alloc_fn, NULL);
    }

    virtual void TearDown ()
    {
        r_state_free (r);
    }

    rsexp read_seq (rsexp port)
    {
        rsexp reader  = r_reader_new (r, port);
        rsexp program = R_NULL;

        while (true) {
            rsexp datum = r_read (reader);

            if (r_eof_object_p (datum))
                break;

            program = r_cons (r, datum, program);
        }

        return program;
    }

    rsexp read_seq (rconstcstring source)
    {
        rsexp str  = r_string_new (r, source);
        rsexp port = r_open_input_string (r, str);

        return read_seq (port);
    }

    rsexp read (rsexp port)
    {
        return r_read (r_reader_new (r, port));
    }

    rsexp read (rconstcstring input)
    {
        rsexp port = r_open_input_string (r, r_string_new (r, input));
        return read (port);
    }

    rsexp compile (rsexp program)
    {
        return r_compile (r, program);
    }

    rconstcstring to_cstr (rsexp obj)
    {
        rsexp out = r_open_output_string (r);
        r_port_write (r, out, obj);
        return r_string_to_cstr (r_get_output_string (r, out));
    }

    testing::AssertionResult eq_p (rsexp lhs, rsexp rhs)
    {
        return r_eq_p (r, lhs, rhs)
               ? testing::AssertionSuccess ()
               : testing::AssertionFailure ();
    }

    testing::AssertionResult eqv_p (rsexp lhs, rsexp rhs)
    {
        return r_eqv_p (r, lhs, rhs)
               ? testing::AssertionSuccess ()
               : testing::AssertionFailure ();
    }

    testing::AssertionResult equal_p (rsexp lhs, rsexp rhs)
    {
        return r_equal_p (r, lhs, rhs)
               ? testing::AssertionSuccess ()
               : testing::AssertionFailure ();
    }

    class allocator_disruptor {
    public:
        allocator_disruptor ()
        {
            fail_alloc = true;
        }

        ~allocator_disruptor ()
        {
            fail_alloc = false;
        }
    };

private:
    static bool fail_alloc;

    static rpointer alloc_fn (rpointer aux, rpointer ptr, rsize size)
    {
        assert (!(ptr == 0 && size == 0u));

        if (fail_alloc)
            return NULL;

        if (0 == size) {
            free (ptr);
            return NULL;
        }

        if (NULL == ptr)
            return malloc (size);

        return realloc (ptr, size);
    }

};

#endif // __ROSE_TEST_UTILS_HPP__
