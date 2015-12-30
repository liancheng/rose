#ifndef __ROSE_READER_H__
#define __ROSE_READER_H__

#include "rose/sexp.h"
#include "rose/state.h"

/// \cond
R_BEGIN_DECLS
/// \endcond

rsexp r_reader_new (RState* r, rsexp port);

rsexp r_read (rsexp reader);

rsexp r_read_from_string (RState* r, rconstcstring input);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __ROSE_READER_H__ */
