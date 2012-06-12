#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/sexp.h"

#define r_symbol_p(s) (((s) & R_TC5_MASK) == R_SYMBOL_TAG)

typedef struct RSymbolTable RSymbolTable;

RSymbolTable* r_symbol_table_new  ();
rsexp         r_symbol_new        (char const* symbol,
                                   RContext*   context);
rsexp         r_symbol_new_static (char const* symbol,
                                   RContext*   context);
char const*   r_symbol_name       (rsexp       obj,
                                   RContext*   context);

#define R_CACHED_SYMBOL(var, name, context)\
        static rsexp var = R_SEXP_FALSE;\
        \
        if (r_false_p (var)) {\
            var = r_symbol_new_static (name, context);\
        }

#endif  //  __ROSE_SYMBOL_H__
