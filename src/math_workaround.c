/**
 * This file is added as an workaround to resolve type name confliction between
 * math.h and rose/types.h.
 */

#include <math.h>

double r_sin (double x)
{
    return sin (x);
}

double r_cos (double x)
{
    return cos (x);
}

double r_ceil (double x)
{
    return ceil (x);
}
