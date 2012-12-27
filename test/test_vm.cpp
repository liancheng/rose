#include "utils.hpp"

#include "rose/rose.h"

class test_vm : public fixture_base {
protected:
    rsexp run_script (rconstcstring filename)
    {
        return r_run (state, rc (r_open_input_file (state, filename)));
    }
};

TEST_F (test_vm, kons)
{
    rsexp expected = r_uint_to_sexp (2u);
    rsexp actual = run_script ("script/kons.scm");

    EXPECT_TRUE (r_eq_p (state, expected, actual));
}

TEST_F (test_vm, call_cc)
{
    rsexp expected = r_uint_to_sexp (2u);
    rsexp actual = run_script ("script/call-cc.scm");

    EXPECT_TRUE (r_eq_p (state, expected, actual));
}
