#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/context.h"
#include "rose/sexp.h"

#define SEXP_SYMBOL_P(s)    (((s) & 0x07) == SEXP_SYMBOL_TAG)

typedef rword rquark;

rquark      r_quark_from_symbol        (char const* symbol,
                                        RContext*   context);
rquark      r_quark_from_static_symbol (char const* symbol,
                                        RContext*   context);
char const* r_quark_to_symbol          (rquark      quark,
                                        RContext*   context);
rsexp       r_sexp_from_symbol         (char const* symbol,
                                        RContext*   context);
rsexp       r_sexp_from_static_symbol  (char const* symbol,
                                        RContext*   context);
char const* r_sexp_to_symbol           (rsexp       sexp,
                                        RContext*   context);

#endif  //  __ROSE_SYMBOL_H__
