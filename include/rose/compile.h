#ifndef __ROSE_COMPILE_H__
#define __ROSE_COMPILE_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

/**
 * Compiles a `program` represented as a standard list to executable VM code.
 */
rsexp r_compile (RState* r, rsexp program);

/**
 * Compiles code snippet read from a given input port.
 */
rsexp r_compile_from_port (RState* r, rsexp port);

/**
 * Compiles code snippet represented as a C-string.
 */
rsexp r_compile_from_cstr (RState* r, rconstcstring input);

R_END_DECLS

#endif /* __ROSE_COMPILE_H__ */
