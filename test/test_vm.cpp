#include "utils.hpp"

#include "rose/eq.h"
#include "rose/number.h"
#include "rose/vm.h"

class test_vm : public fixture_base {
protected:
    rsexp run_script (rconstcstring filename)
    {
        rsexp port = r_open_input_file (r, filename);
        rsexp code = r_compile_from_port (r, port);
        return r_eval (r, code);
    }
};

TEST_F (test_vm, kons)
{
    rsexp expected = r_uint_to_sexp (2u);
    rsexp actual = run_script ("script/kons.scm");

    EXPECT_TRUE (eq_p (expected, actual));
}

TEST_F (test_vm, call_cc)
{
    rsexp expected = r_uint_to_sexp (2u);
    rsexp actual = run_script ("script/call-cc.scm");

    EXPECT_TRUE (eq_p (expected, actual));
}

TEST_F (test_vm, factorial)
{
    rsexp expected = r_uint_to_sexp (3628800u);
    rsexp actual = run_script ("script/factorial.scm");

    EXPECT_TRUE (eq_p (expected, actual));
}
