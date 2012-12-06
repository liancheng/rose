#include <gc/gc.h>
#include <gtest/gtest.h>

int main (int argc, char* argv[])
{
    GC_INIT ();
    testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
