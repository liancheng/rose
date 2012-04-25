#ifndef __ROSE_CONTEXT_H__
#define __ROSE_CONTEXT_H__

#include "rose/scanner_types.h"

#include <glib.h>

typedef struct r_context {
    r_scanner* scanner;
}
r_context;

r_context* context_new  ();
void       context_free (r_context* context);

#endif  //  __ROSE_CONTEXT_H__
