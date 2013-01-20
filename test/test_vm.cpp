#include "utils.hpp"

#include "rose/eq.h"
#include "rose/io.h"
#include "rose/number.h"
#include "rose/vm.h"

class test_vm : public fixture_base {
protected:
    void SetUp ()
    {
        fixture_base::SetUp ();
        output_port = r_open_output_string (r);
    }

    rsexp run_script (rconstcstring filename)
    {
        return run_script (filename, "");
    }

    rsexp run_script (rconstcstring filename, rconstcstring input)
    {
        rsexp port = r_open_input_file (r, filename);
        rsexp code = r_compile_from_port (r, port);
        rsexp str = r_string_new (r, input);

        r_set_current_input_port_x (r, r_open_input_string (r, str));
        r_set_current_output_port_x (r, output_port);

        return r_eval (r, code);
    }

    rconstcstring get_output ()
    {
        return r_string_to_cstr (r_get_output_string (r, output_port));
    }

private:
    rsexp output_port;
};

TEST_F (test_vm, kons)
{
    EXPECT_TRUE (eq_p (R_UNSPECIFIED, run_script ("script/kons.scm")));
    EXPECT_STREQ ("1 2", get_output ());
}

TEST_F (test_vm, call_cc)
{
    EXPECT_TRUE (eq_p (r_uint_to_sexp (2u),
                       run_script ("script/call-cc.scm")));
}

TEST_F (test_vm, factorial)
{
    EXPECT_TRUE (eq_p (R_UNSPECIFIED,
                       run_script ("script/factorial.scm",
                                   "100\n")));

    EXPECT_STREQ ("9332621544394415268169923885626670049071"
                  "5968264381621468592963895217599993229915"
                  "6089414639761565182862536979208272237582"
                  "51185210916864000000000000000000000000\n",
                  get_output ());
}
