#include <gtest/gtest.h>
#include <mcheck.h>

#include <libgen.h>
#include <unistd.h>

#include <iostream>

using namespace std;

int main (int argc, char* argv[])
{
    mtrace ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
