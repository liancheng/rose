#include "detail/sexp_io.h"
#include "rose/context.h"
#include "rose/pair.h"
#include "rose/port.h"
#include "rose/string.h"
#include "rose/symbol.h"
#include "rose/vector.h"
#include "rose/writer.h"

void r_write (rsexp port, rsexp obj, RContext* context)
{
    if (R_SEXP_TRUE == obj) {
        r_port_puts (port, "#t");
    }
    else if (R_SEXP_FALSE == obj) {
        r_port_puts (port, "#f");
    }
    else if (r_unspecified_p (obj)) {
        r_port_puts (port, "#<unspecified>");
    }
    else if (r_symbol_p (obj)) {
        r_write_symbol (port, obj, context);
    }
    else if (r_pair_p (obj)) {
        r_write_pair (port, obj, context);
    }
    else if (r_null_p (obj)) {
        r_write_null (port, obj, context);
    }
    else if (r_string_p (obj)) {
        r_write_string (port, obj, context);
    }
    else if (r_vector_p (obj)) {
        r_write_vector (port, obj, context);
    }
    else if (r_port_p (obj)) {
        r_write_port (port, obj, context);
    }
}

void r_display (rsexp port, rsexp obj, RContext* context)
{
    if (R_SEXP_TRUE == obj) {
        r_port_puts (port, "#t");
    }
    else if (R_SEXP_FALSE == obj) {
        r_port_puts (port, "#f");
    }
    else if (r_symbol_p (obj)) {
        r_display_symbol (port, obj, context);
    }
    else if (r_pair_p (obj)) {
        r_display_pair (port, obj, context);
    }
    else if (r_null_p (obj)) {
        r_display_null (port, obj, context);
    }
    else if (r_string_p (obj)) {
        r_display_string (port, obj, context);
    }
    else if (r_vector_p (obj)) {
        r_display_vector (port, obj, context);
    }
    else if (r_port_p (obj)) {
        r_display_port (port, obj, context);
    }
}
