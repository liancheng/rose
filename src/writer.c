#include "detail/sexp.h"
#include "rose/state.h"
#include "rose/port.h"

void r_write (rsexp port, rsexp obj)
{
    RState* state = r_port_get_state (port);
    RTypeDescriptor* type = r_describe (state, obj);

    if (type)
        type->ops.write (port, obj);
}

void r_display (rsexp port, rsexp obj)
{
    RState* state = r_port_get_state (port);
    RTypeDescriptor* type = r_describe (state, obj);

    if (type)
        type->ops.display (port, obj);
}
