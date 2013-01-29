#include "rose/env.h"

#include "utils.hpp"

class test_env : public fixture_base {
protected:
    virtual void SetUp ()
    {
        fixture_base::SetUp ();
        env = r_empty_env (r);
    }

    rsexp symbol (rconstcstring name)
    {
        return r_intern (r, name);
    }

    rsexp assign_x (rconstcstring var, rsexp val)
    {
        rsexp e;
        ensure (e = r_env_assign_x (r, env, symbol (var), val));
        return env = e;
    }

    rsexp bind_x (rconstcstring var, rsexp val)
    {
        rsexp e;
        ensure (e = r_env_bind_x (r, env, symbol (var), val));
        return env = e;
    }

    rsexp lookup (rconstcstring var)
    {
        rsexp vals = r_env_lookup (r, env, symbol (var));
        return r_undefined_p (vals) ? R_UNDEFINED : r_car (vals);
    }

    rsexp env;
};

TEST_F (test_env, r_env_bind_x)
{
    EXPECT_FALSE (r_failure_p (bind_x ("x", R_FALSE)));
    EXPECT_TRUE (eq_p (R_FALSE, lookup ("x")));

    EXPECT_FALSE (r_failure_p (bind_x ("x", R_TRUE)));
    EXPECT_TRUE (eq_p (R_TRUE, lookup ("x")));
}

TEST_F (test_env, r_env_assign_x)
{
    EXPECT_TRUE (r_failure_p (assign_x ("x", R_FALSE)));

    EXPECT_FALSE (r_failure_p (bind_x ("x", R_FALSE)));
    EXPECT_FALSE (r_failure_p (assign_x ("x", R_TRUE)));
    EXPECT_TRUE (eq_p (R_TRUE, lookup ("x")));
}
