#ifndef __ROSE_SYMBOL_H__
#define __ROSE_SYMBOL_H__

#include "rose/context.h"
#include "rose/sexp.h"

#define SEXP_SYMBOL_P(s)    (((s) & 0x07) == SEXP_SYMBOL_TAG)

typedef r_word r_quark;

r_quark     quark_from_symbol        (char const* symbol,
                                      r_context*  context);
r_quark     quark_from_static_symbol (char const* symbol,
                                      r_context*  context);
char const* quark_to_symbol          (r_quark     quark,
                                      r_context*  context);
r_sexp      sexp_from_symbol         (char const* symbol,
                                      r_context*  context);
r_sexp      sexp_from_static_symbol  (char const* symbol,
                                      r_context*  context);
char const* sexp_to_symbol           (r_sexp      sexp,
                                      r_context*  context);

#endif  //  __ROSE_SYMBOL_H__
