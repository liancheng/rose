#ifndef __ROSE_TEST_UTILS_HPP__
#define __ROSE_TEST_UTILS_HPP__

#include "rose/state.h"

#include <gtest/gtest.h>

class fixture_base : public testing::Test {
protected:
    virtual void SetUp ()
    {
        state = r_state_open ();
    }

    virtual void TearDown ()
    {
        r_state_free (state);
    }

    RState* state;
};

#endif  //  __ROSE_TEST_UTILS_HPP__
