#include "utils.hpp"

#include "rose/eq.h"
#include "rose/number.h"
#include "rose/vm.h"

class test_vm : public fixture_base {
protected:
    rsexp run_script (rconstcstring filename)
    {
        rsexp port = r_open_input_file (r, filename);
        rsexp dag = r_compile_from_port (r, port);
        return r_run (r, dag);
    }
};

TEST_F (test_vm, kons)
{
    rsexp expected = r_uint_to_sexp (2u);
    rsexp actual = run_script ("script/kons.scm");

    EXPECT_TRUE (r_eq_p (r, expected, actual));
}

TEST_F (test_vm, call_cc)
{
    rsexp expected = r_uint_to_sexp (2u);
    rsexp actual = run_script ("script/call-cc.scm");

    EXPECT_TRUE (r_eq_p (r, expected, actual));
}
