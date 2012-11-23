#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/sexp.h"

#define r_symbol_p(obj)     (R_GET_TAG (obj) == R_SYMBOL_TAG)

typedef struct RSymbolTable RSymbolTable;

RSymbolTable* r_symbol_table_new  (RState*       state);
rsexp         r_symbol_new        (RState*       state,
                                   char const*   symbol);
rsexp         r_symbol_new_static (RState*       state,
                                   char const*   symbol);
void          r_symbol_table_free (RState*       state,
                                   RSymbolTable* symbol_table);
char const*   r_symbol_name       (RState*       state,
                                   rsexp         obj);

#endif  /* __ROSE_SYMBOL_H__ */
