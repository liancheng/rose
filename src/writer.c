#include "detail/sexp.h"
#include "rose/context.h"
#include "rose/port.h"

void r_write (rsexp port, rsexp obj)
{
    RContext* context = r_port_get_context (port);
    RType* type = r_sexp_get_type (obj, context);

    if (type)
        (type->write_fn) (port, obj);
}

void r_display (rsexp port, rsexp obj)
{
    RContext* context = r_port_get_context (port);
    RType* type = r_sexp_get_type (obj, context);

    if (type)
        (type->display_fn) (port, obj);
}
