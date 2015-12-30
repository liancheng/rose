#ifndef __ROSE_DETAIL_SSTREAM_H__
#define __ROSE_DETAIL_SSTREAM_H__

#ifdef __APPLE__

#include "rose/types.h"

#include <stdio.h>

/// \cond
R_BEGIN_DECLS
/// \endcond

FILE* fmemopen (void* buf, size_t size, char const* mode);
FILE* open_memstream (char** buf_ptr, size_t* size_ptr);

/// \cond
R_END_DECLS
/// \endcond

#endif /* __APPLE__ */

#endif /* __ROSE_DETAIL_SSTREAM_H__ */
