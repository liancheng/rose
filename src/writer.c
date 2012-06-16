#include "detail/sexp.h"
#include "rose/context.h"

void r_write (rsexp port, rsexp obj, RContext* context)
{
    RType* type = r_sexp_get_type (obj, context);

    if (type)
        (type->write_fn) (port, obj, context);
}

void r_display (rsexp port, rsexp obj, RContext* context)
{
    RType* type = r_sexp_get_type (obj, context);

    if (type)
        (type->display_fn) (port, obj, context);
}
