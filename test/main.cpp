#include <glib.h>

void add_vector_test_suite ();
void add_number_test_suite ();

int main (int argc, char* argv[])
{
    g_test_init (&argc, &argv, NULL);

    add_number_test_suite ();
    add_vector_test_suite ();

    return g_test_run ();
}
