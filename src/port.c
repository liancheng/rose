#include "boxed.h"

#include "rose/port.h"

rboolean r_port_p(rsexp sexp)
{
    return R_BOXED_P(sexp) && r_boxed_get_type(sexp) == SEXP_PORT;
}
