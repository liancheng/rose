#ifndef __ROSE_CONTEXT_H__
#define __ROSE_CONTEXT_H__

#include "rose/scanner_types.h"

#include <glib.h>

typedef struct RContext {
    RScanner* scanner;
}
RContext;

RContext* r_context_new  ();
void      r_context_free (RContext* context);

#endif  //  __ROSE_CONTEXT_H__
