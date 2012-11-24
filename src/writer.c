#include "detail/sexp.h"
#include "rose/state.h"
#include "rose/port.h"

void r_write (rsexp port, rsexp obj)
{
    RState* state = r_port_get_state (port);
    RTypeInfo* type_info = r_type_info (state, obj);

    if (type_info)
        type_info->ops.write (port, obj);
}

void r_display (rsexp port, rsexp obj)
{
    RState* state = r_port_get_state (port);
    RTypeInfo* type_info = r_type_info (state, obj);

    if (type_info)
        type_info->ops.display (port, obj);
}
