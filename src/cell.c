#include "cell.h"

#include <assert.h>

RCellType r_cell_get_type (rsexp obj)
{
    assert (r_cell_p (obj));
    return ((RCell*) obj)->type;
}

void r_cell_set_type (rsexp obj, RCellType type)
{
    assert (r_cell_p (obj));
    ((RCell*) obj)->type = type;
}
