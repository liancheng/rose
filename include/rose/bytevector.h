#ifndef __ROSE_BYTEVECTOR_H__
#define __ROSE_BYTEVECTOR_H__

#include "rose/sexp.h"
#include "rose/state.h"

R_BEGIN_DECLS

/**
 * Creates a new bytevector with length `k` and filled with `fill`.
 *
 * Equivalent to standard scheme procedure `new-bytevector`.
 */
rsexp r_bytevector_new (RState* r, rsize k, rbyte fill);

/**
 * Determines whether `obj` is a bytevector.
 *
 * Equivalent to standard schema procedure `bytevector?`.
 */
rbool r_bytevector_p (rsexp obj);

/**
 * Returns the length of `obj` if it's a bytevector.
 *
 * Equivalent to standard schema procedure `bytevector-length`.
 */
rsexp r_bytevector_length (rsexp obj);

/**
 * Returns the `k`-th unsigned byte in bytevector `obj`.
 *
 * Equivalent to standard schema procedure `bytevector-u8-ref`.
 */
rsexp r_bytevector_u8_ref (RState* r, rsexp obj, rsize k);

/**
 * Sets the `k`-th element of bytevector `obj` to unsigned byte `byte`.
 *
 * Equivalent to standard schema procedure `bytevector-u8-set!`.
 */
rsexp r_bytevector_u8_set_x (RState* r, rsexp obj, rsize k, rbyte byte);

/**
 * Converts a list of unsigned bytes into a bytevector.
 *
 * Equivalent to standard schema procedure `list->bytevector`.
 */
rsexp r_list_to_bytevector (RState* r, rsexp list);

R_END_DECLS

#endif /* __ROSE_BYTEVECTOR_H__ */
