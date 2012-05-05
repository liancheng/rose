#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/sexp.h"

#include <stdio.h>

#define r_symbol_p(s) (((s) & 0x07) == R_SEXP_SYMBOL_TAG)

typedef struct RSymbolTable RSymbolTable;

RSymbolTable* r_symbol_table_new  ();
rsexp         r_symbol_new        (char const* symbol,
                                   rsexp       context);
rsexp         r_symbol_new_static (char const* symbol,
                                   rsexp       context);
char const*   r_symbol_name       (rsexp       sexp,
                                   rsexp       context);
void          r_write_symbol      (rsexp       output,
                                   rsexp       sexp,
                                   rsexp       context);
rsexp         r_read_symbol       (rsexp       input,
                                   rsexp       context);

#endif  //  __ROSE_SYMBOL_H__
