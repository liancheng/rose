#include "utils.hpp"

#include <gtest/gtest.h>
#include <mcheck.h>

#include <libgen.h>
#include <unistd.h>

#include <iostream>

using namespace std;

bool fixture_base::fail_alloc = false;

int main (int argc, char* argv[])
{
    mtrace ();

    chdir (dirname (argv [0]));
    chdir ("../../test");
    testing::InitGoogleTest (&argc, argv);

    return RUN_ALL_TESTS ();
}
