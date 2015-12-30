#ifndef __ROSE_VM_H__
#define __ROSE_VM_H__

#include "rose/sexp.h"
#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

/**
 * Evaluates a compiled instruction sequence stored in `code`.
 *
 * \pre `code` points to a compiled instruction sequence
 * \post `r->vm->next` stores the final result of `code`
 */
rsexp r_eval (RState* r, rsexp code);

/**
 * Evaluates a Scheme program read from input port `port`.
 *
 * \pre `port` is an input port
 * \post `r->vm->next` stores the final result of the program
 */
rsexp r_eval_from_port (RState* r, rsexp port);

/**
 * Evaluates a Scheme program stored in input string `input`.
 *
 * \pre `input` is a Scheme string object containing a Scheme program
 * \post `r->vm->next` stores the final result of the program
 */
rsexp r_eval_from_string (RState* r, rsexp input);

/**
 * Evaluates a Scheme program stored in input C-string `input`.
 *
 * \pre `input` is null-terminated C-string containing a Scheme program
 * \post `r->vm->next` stores the final result of the program
 */
rsexp r_eval_from_cstr (RState* r, rconstcstring input);

/**
 * Evaluates a Scheme program stored in a source file at path `path`.
 *
 * \pre `path` is null-terminated C-string containing the source file path
 * \post `r->vm->next` stores the final result of the program
 */
rsexp r_eval_from_file (RState* r, rconstcstring path);

/// \cond
R_END_DECLS
/// \endcond

#endif /*  __ROSE_VM_H__ */
