#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/sexp.h"

#define r_symbol_p(obj) R_TC5_EQ_P ((obj), R_SYMBOL_TAG)

typedef struct RSymbolTable RSymbolTable;

RSymbolTable* r_symbol_table_new  ();
rsexp         r_symbol_new        (char const* symbol,
                                   RContext*   context);
rsexp         r_symbol_new_static (char const* symbol,
                                   RContext*   context);
char const*   r_symbol_name       (rsexp       obj,
                                   RContext*   context);

#endif  //  __ROSE_SYMBOL_H__
