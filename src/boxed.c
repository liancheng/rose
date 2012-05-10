#include "boxed.h"

#include <assert.h>

RBoxedType r_boxed_get_type (rsexp obj)
{
    assert (r_boxed_p (obj));
    return ((RBoxed*) obj)->type;
}

void r_boxed_set_type (rsexp obj, RBoxedType type)
{
    assert (r_boxed_p (obj));
    ((RBoxed*) obj)->type = type;
}
