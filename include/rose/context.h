#ifndef __ROSE_CONTEXT_H__
#define __ROSE_CONTEXT_H__

#include "rose/scanner_types.h"
#include "rose/sexp.h"

typedef struct RContext RContext;

RContext* r_context_new            ();
void      r_context_free           (RContext* context);
RScanner* r_context_get_scanner    (RContext* context);
rsexp     r_context_get_global_env (RContext* context);

#endif  //  __ROSE_CONTEXT_H__
