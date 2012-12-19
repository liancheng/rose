#include <gtest/gtest.h>
#include <mcheck.h>

int main (int argc, char* argv[])
{
    mtrace ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
